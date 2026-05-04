#pragma once

/**
 * @file    buffer.hpp
 * @author  dexus1337
 * @brief   Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include <memory>
#include <atomic>
#include <cstdint>

#include "string/string-hashed.hpp"

namespace adam 
{
    class memory;
    class buffer_manager;

    /**
     * @struct buffer_handle
     * @brief A lightweight, POD structure used to safely send a buffer's location across IPC queues.
     */
    struct buffer_handle
    {
        uint64_t memory_index;      /**< The globally unique index of the shared memory segment. */
        uint32_t offset;            /**< The offset within the shared memory segment. */
        uint32_t size;              /**< The size of the buffer payload. */
    };

    /**
     * @class buffer
     * @brief Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
     */
    class ADAM_SDK_API buffer 
    {
        friend class buffer_manager;
        friend struct std::default_delete<buffer>;
        friend struct std::default_delete<buffer[]>;

    public:

        /** @brief Accessor to the mapped data pointer in the current process. */
        void* get_data() const { return m_data; }

        /** @brief Accessor to the capacity of the buffer. */
        uint32_t get_capacity() const { return m_capacity; }

        /** @brief Adds a reference to the buffer. */
        void add_ref() { m_ref_count->fetch_add(1, std::memory_order_relaxed); }
        
        /** @brief Releases a reference, returning it to the manager if it reaches 0. */
        void release();

        /** @brief IPC support: Creates a lightweight handle safe for cross-process transmission. */
        buffer_handle get_handle() const;

    protected:

        /**
         * @brief Constructs a new buffer object.
         */
        buffer();

        /**
         * @brief Destroys the buffer object and cleans up resources.
         */
        ~buffer();

        std::atomic<uint32_t>* m_ref_count; /**< Pointer to the shared reference count. */
        void* m_data;                       /**< Pointer to the actual memory buffer data. */
        uint32_t m_capacity;                /**< The capacity of the memory buffer in bytes. */
        uint32_t m_memory_index;            /**< The unique index of the shared memory instance hosting this buffer. */
        uint32_t m_offset;                  /**< The offset inside the shared memory instance. */
    };
}