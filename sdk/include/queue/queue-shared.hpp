#pragma once

/**
 * @file        queue_type-queue.hpp
 * @author      dexus1337
 * @brief       Defines a interprocess queue for any datatype, based on shared memory
 * @version     1.0
 * @date        27.04.2026
 */

 
#include "api/api.hpp"

#include <atomic>
#include <cstddef>

#include "string/string-hashed.hpp"
#include "memory/shared/memory-shared.hpp"

namespace adam 
{
    /**
     * @class queue_shared
     * @brief Defines a interprocess queue for any datatype, based on shared memory
     */
    template< typename queue_type >
    class ADAM_SDK_API queue_shared
    {
    public:

        /** @brief Constructs a new queue_shared object.*/
        queue_shared(const string_hashed& name);

        /** @brief Destroys the queue_shared object and cleans up resources.*/
        ~queue_shared();

        /** @brief Gives the amount of items being able to be queued simoultanesously */
        uint32_t get_max_items() const { return m_shared_memory.get() ? this->get_header()->max_items : 0; }

        /** @brief Checks wether the queue is full */
        bool is_full() const;

        /** @brief Checks wether the queue is empty, aka, anything to pull */
        bool is_empty() const;

        /** @brief Creates the queue_type queue and the underlying shared_memory for managing a max amount of items given */
        bool create(uint32_t max_items);

        /** @brief Opens an existing queue_type queue */
        bool open();

        /** @brief Destroys the queue and free all resources */
        bool destroy();

        /** @brief Pushes a object in the list and notifies the signal */
        bool push(const queue_type& obj);

        /** @brief Waits for available items and pops in fifo order */
        bool pop(queue_type& out_obj, int32_t timeout = -1);

        /** @brief Pushes an object in the list and notifies the signal. Compatible with std::chrono::duration */
        template <typename rep, typename period>
        bool pop(queue_type& out_obj, std::chrono::duration<rep, period> timeout = std::chrono::duration<rep, period>::max()) { return pop(out_obj, timeout == std::chrono::duration<rep, period>::max() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()); }

    protected:

        struct queue_header
        {
            uint32_t max_items;

            // We use a circular queue for max performance here
            std::atomic<uint32_t> head; /**< The index of the head of the queue_type queue, used for tracking where new items should be added. */
            std::atomic<uint32_t> tail; /**< The index of the tail of the queue_type queue, used for tracking where items should be read from. */
        
            queue_type items[1];
        };

        /** @brief Accessor for header from inside the shared memory */
        queue_header* header() { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        /** @brief Const Accessor for header from inside the shared memory */
        const queue_header* get_header() const { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        memory_shared m_shared_memory;
    };

    template< typename queue_type >
    queue_shared<queue_type>::queue_shared(const string_hashed& name)
     :  m_shared_memory(name)
    {
    }

    template< typename queue_type >
    queue_shared<queue_type>::~queue_shared()
    {
        destroy();
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::is_full() const
    {
        const queue_header* h = get_header();

        if (!h) 
            return false;

        // to safely call this from owner and observer, use acquire for both. Slows down stuff so only use when needed
        uint32_t head = h->head.load(std::memory_order_acquire);
        uint32_t tail = h->tail.load(std::memory_order_acquire);

        // Full is defined as: (Head + 1) % Max == Tail
        // This is why one slot is always left empty.
        return ((head + 1) % h->max_items) == tail;
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::is_empty() const
    {
        const queue_header* h = get_header();

        if (!h) 
            return false;

        // Empty is simple: Head and Tail are at the same position.
        // to safely call this from owner and observer, use acquire for both. Slows down stuff so only use when needed
        return h->head.load(std::memory_order_acquire) == h->tail.load(std::memory_order_acquire);
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::create(uint32_t max_items)
    {
        if (max_items == 0)
            return false;

        // Calculate total size: queue_header already contains 1 queue_type, so we add space for max_items - 1
        uint64_t buffer_size = sizeof(queue_header) + sizeof(queue_type) * (max_items - 1);

        if (!m_shared_memory.create(buffer_size))
            return false;

        // Initialize queue_header and items in shared memory using placement new
        queue_header* header = reinterpret_cast<queue_header*>(m_shared_memory.get());
        new (&header->head) std::atomic<uint32_t>(0);
        new (&header->tail) std::atomic<uint32_t>(0);
        
        for (uint32_t i = 0; i < max_items; i++)
            new (&header->items[i]) queue_type();

        header->max_items = max_items;

        return true;
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::open()
    {
        return m_shared_memory.open();
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::destroy()
    {
        return m_shared_memory.destroy();
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::push(const queue_type& object)
    {
        auto* header = this->header();
        
        uint32_t current_head = header->head.load(std::memory_order_relaxed); // we own head at that moment, so use relaxed
        uint32_t next_head = (current_head + 1) % header->max_items;

        if (next_head == header->tail.load(std::memory_order_acquire)) // tail is not owned by us rn, thats why he have to acquire
            return false; // Queue full

        header->items[current_head] = object;
        header->head.store(next_head, std::memory_order_release);
        
        return m_shared_memory.signal().notify();
    }

    template< typename queue_type >
    bool queue_shared<queue_type>::pop(queue_type& out_object, int32_t timeout)
    {
        // Wait for a signal that data is available
        if (!m_shared_memory.signal().wait(timeout))
            return false; // Timeout

        auto* header = this->header();
        uint32_t current_tail = header->tail.load(std::memory_order_relaxed); // we own tail at that moment, so use relaxed
        
        if (current_tail == header->head.load(std::memory_order_acquire)) // head is not owned by us rn, thats why he have to acquire
            return false; // Spurious wake-up / Empty

        out_object = header->items[current_tail];
        header->tail.store((current_tail + 1) % header->max_items, std::memory_order_release);
        
        return true;
    }
}