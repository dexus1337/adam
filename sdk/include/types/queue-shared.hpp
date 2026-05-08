#pragma once

/**
 * @file    queue-shared.hpp
 * @author  dexus1337
 * @brief   Defines a interprocess queue for any datatype, based on shared memory
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"

#include <atomic>
#include <cstddef>
#include <chrono>

#include "types/string-hashed.hpp"
#include "memory/memory-signaled.hpp"

namespace adam 
{
    /**
     * @class queue_shared
     * @brief Defines a interprocess queue for any datatype, based on shared memory
     */
    template<typename queue_type, typename queue_metadata_type = uint32_t>
    class queue_shared
    {
    public:

        /** @brief Constructs a new queue_shared object.*/
        queue_shared(const string_hashed& name = string_hashed());

        /** @brief Destroys the queue_shared object and cleans up resources.*/
        ~queue_shared();

        /** @brief Gives the amount of items being able to be queued simoultanesously */
        uint32_t get_max_items() const { return m_shared_memory.get() ? this->get_header()->max_items : 0; }

        /** @brief Const Accessor for metadata inside the header from inside the shared memory */
        const queue_metadata_type* get_metadata() const { return get_header() ? &get_header()->metadata : nullptr; }

        /** @brief Checks wether the queue is full. */
        bool is_full() const;

        /** @brief Checks wether the queue is empty, aka, anything to pull. */
        bool is_empty() const;

        /** @brief Gets the memorys active flag. Can be as loop condition for threads .*/
        bool is_active() const { return m_shared_memory.is_active(); };

        void set_name(const string_hashed& new_name) { m_shared_memory.set_name(new_name); }

        /** @brief Creates the queue_type queue and the underlying shared_memory for managing a max amount of items given. */
        bool create(uint32_t max_items);

        /** @brief Opens an existing queue_type queue. */
        bool open();

        /** @brief Unsets the active flag. Can be used to stop waiting threads. */
        void disable() { m_shared_memory.disable(); }

        /** @brief Destroys the queue and free all resources. */
        bool destroy();

        /** @brief Pushes a object in the list and notifies the signal. */
        bool push(const queue_type& obj);

        /** @brief Waits for available items and pops in fifo order. */
        bool pop(queue_type& out_obj, int32_t timeout = -1);

        /** @brief Pushes an object in the list and notifies the signal. Compatible with std::chrono::duration */
        template <typename rep, typename period>
        bool pop(queue_type& out_obj, std::chrono::duration<rep, period> timeout = std::chrono::duration<rep, period>::max()) { return pop(out_obj, timeout == std::chrono::duration<rep, period>::max() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()); }

        /** @brief Accessor for metadata inside the header from inside the shared memory */
        queue_metadata_type* metadata() { return header() ? header()->metadata : nullptr; }

    protected:

        struct queue_header
        {
            uint32_t max_items;

            queue_metadata_type metadata;

            // We use a circular queue for max performance here
            std::atomic<uint32_t> head; /**< The index of the head of the queue_type queue, used for tracking where new items should be added. */
            std::atomic<uint32_t> tail; /**< The index of the tail of the queue_type queue, used for tracking where items should be read from. */
        
            queue_type items[1];
        };

        /** @brief Accessor for header from inside the shared memory */
        queue_header* header() { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        /** @brief Const Accessor for header from inside the shared memory */
        const queue_header* get_header() const { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        memory_signaled m_shared_memory;
    };

    template<typename queue_type, typename queue_metadata_type>
    queue_shared<queue_type, queue_metadata_type>::queue_shared(const string_hashed& name)
     :  m_shared_memory(name)
    {
    }

    template<typename queue_type, typename queue_metadata_type>
    queue_shared<queue_type, queue_metadata_type>::~queue_shared()
    {
        destroy();
    }

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::is_full() const
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

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::is_empty() const
    {
        const queue_header* h = get_header();

        if (!h) 
            return false;

        // Empty is simple: Head and Tail are at the same position.
        // to safely call this from owner and observer, use acquire for both. Slows down stuff so only use when needed
        return h->head.load(std::memory_order_acquire) == h->tail.load(std::memory_order_acquire);
    }

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::create(uint32_t max_items)
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

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::open()
    {
        return m_shared_memory.open();
    }

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::destroy()
    {
        return m_shared_memory.destroy();
    }

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::push(const queue_type& object)
    {
        auto* header = this->header();
        
        uint32_t current_head = header->head.load(std::memory_order_relaxed);
        uint32_t next_head = (current_head + 1) % header->max_items;

        if (next_head == header->tail.load(std::memory_order_acquire))
            return false; // Queue full

        header->items[current_head] = object;
        header->head.store(next_head, std::memory_order_release);
        
        return m_shared_memory.notify();
    }

    template<typename queue_type, typename queue_metadata_type>
    bool queue_shared<queue_type, queue_metadata_type>::pop(queue_type& out_object, int32_t timeout)
    {
        auto* header = this->header();
        
        if (timeout >= 0)
        {
            auto end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
            
            while (header->tail.load(std::memory_order_relaxed) == header->head.load(std::memory_order_acquire)) // Wait while queue empty
            {
                auto now = std::chrono::steady_clock::now();
                if (now >= end_time)
                    return false; // Timeout reached

                int32_t remaining = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - now).count());
                
                if (!m_shared_memory.wait(remaining))
                    return false; // OS level timeout or failure
            }
        }
        else
        {
            // Infinite wait
            while (header->tail.load(std::memory_order_relaxed) == header->head.load(std::memory_order_acquire)) // Wait while queue empty
            {
                if (!m_shared_memory.wait(-1))
                    return false; // Error during wait
            }
        }

        uint32_t current_tail = header->tail.load(std::memory_order_relaxed);
        out_object = header->items[current_tail];
        header->tail.store((current_tail + 1) % header->max_items, std::memory_order_release);
        
        return true;
    }
}