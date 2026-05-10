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
#include <cstring>

#include "types/string-hashed.hpp"
#include "data/format.hpp"
#include "os/os.hpp"

namespace adam 
{
    class memory;
    class buffer_manager;
    class data_format;

    /**
     * @struct buffer_handle
     * @brief A lightweight, POD structure used to safely send a buffer's location across IPC queues.
     */
    struct buffer_handle
    {
        uint32_t memory_index;      /**< The globally unique index of the shared memory segment. */
        os::thread_id thread_id;    /**< The ID of the thread that created the memory block. */
        uint32_t offset;            /**< The offset within the shared memory segment. */
        uint32_t capacity;          /**< The full capacity of the buffer. */
        uint32_t size;              /**< The size of the buffer payload. */
        uint64_t data_format_hash;  /**< The hash of the name of the data format. */
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

        const void* get_data()                  const { return m_data; }
        uint32_t get_capacity()                 const { return m_capacity; }
        uint32_t get_size()                     const { return m_size; }
        const data_format* get_data_format()    const { return m_data_format; }

        /** @brief IPC support: Creates a lightweight handle safe for cross-process transmission. */
        buffer_handle get_handle() const;

        void set_size(uint32_t size) { m_size = size; }
        void set_data_format(const data_format* format) { m_data_format = format; }

        /** @brief Adds a reference to the buffer. */
        void add_ref() { m_ref_count->fetch_add(1, std::memory_order_relaxed); }
        
        /** @brief Releases a reference, returning it to the manager if it reaches 0. */
        void release();

    protected:

        /** @brief Constructs a new buffer object. */
        buffer();

        /** @brief Destroys the buffer object and cleans up resources. */
        ~buffer();

        std::atomic<uint32_t>* m_ref_count; /**< Pointer to the shared reference count. */
        void* m_data;                       /**< Pointer to the actual memory buffer data. */
        const data_format* m_data_format;   /**< Optional pointer to a data format describing the contents of this buffer, used for automatic parsing/serialization in the ADAM system. */
        uint32_t m_capacity;                /**< The capacity of the memory buffer in bytes. */
        uint32_t m_size;                    /**< The current size of the data in the buffer. */
        uint32_t m_memory_index;            /**< The unique index of the shared memory instance hosting this buffer. */
        os::thread_id m_thread_id;          /**< The thread id that created the shared memory instance hosting this buffer. */
        uint32_t m_offset;                  /**< The offset inside the shared memory instance. */
        bool m_is_resolved;                 /**< Indicates whether this buffer has been resolved from a handle. */
    };
}