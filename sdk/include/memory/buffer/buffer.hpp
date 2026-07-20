#pragma once

/**
 * @file    buffer.hpp
 * @author  dexus1337
 * @brief   Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"

#include <memory>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <chrono>

#include "memory/buffer/buffer-manager.hpp"
#include "module/internals/essential/data/formats/data-format-transparent.hpp"
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
    #pragma pack(push, 1)
    struct ADAM_SDK_API buffer_handle
    {
        uint32_t        memory_index;   /**< The globally unique index of the shared memory segment. */
        uint32_t        offset;         /**< The offset within the shared memory segment. */
        uint32_t        capacity;       /**< The capacity of the buffer. */
        os::thread_id   thread_id;      /**< The ID of the thread that created the memory block. */

        inline bool is_valid() const { return thread_id != 0; }
        inline void set_invalid() { thread_id = 0; }

        buffer_handle() {}
        buffer_handle(uint32_t memory_index, uint32_t offset, uint32_t capacity, os::thread_id thread_id) : memory_index(memory_index), offset(offset), capacity(capacity), thread_id(thread_id) {}
    };
    #pragma pack(pop)

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

        // The reference count takes sizeof(std::atomic<uint32_t>) = 4 bytes, but we pad it to 8 bytes (sizeof(uint64_t))
        // to ensure the subsequent data payload (m_data) is aligned to an 8-byte boundary.
        static constexpr uint32_t ref_count_padding = sizeof(uint64_t);

        template<typename T>
        inline const T* get_data_as()                  const { return reinterpret_cast<const T*>(m_data); }
        template<typename T>
        inline const T* get_begin_as()                 const { return reinterpret_cast<const T*>(get_data_as<uint8_t>() + m_header->start_pos); }
        template<typename T>
        inline const T* get_end_as()                   const { return reinterpret_cast<const T*>(get_data_as<uint8_t>() + m_header->start_pos + m_header->size); }
        template<typename T>
        inline const T* get_at(uint32_t off)           const { return reinterpret_cast<const T*>(get_data_as<uint8_t>() + m_header->start_pos + off); }

        inline const void* get_data()                  const { return m_data; }
        inline const void* get_begin()                 const { return get_begin_as<uint8_t>(); }
        inline const void* get_end()                   const { return get_end_as<uint8_t>(); }
        inline uint32_t get_capacity()                 const { return m_header->capacity; }
        inline uint32_t get_remaining_capacity()       const { return m_header->capacity - (m_header->size + m_header->start_pos); }
        inline uint32_t get_size()                     const { return m_header->size; }
        inline uint32_t get_start_pos()                const { return m_header->start_pos; }
        inline const data_format* get_data_format()    const { return m_data_format; }
        inline uint64_t get_timestamp()                const { return m_header->timestamp; }
        inline uint32_t get_ref_count()                const { return m_header->ref_count.load(std::memory_order_relaxed); }
        inline buffer* get_referenced_buffer()         const { return m_reference; }
        inline const buffer_handle& get_handle()       const { return m_handle; }

        template<typename T>
        inline T* begin_as()                           { return reinterpret_cast<T*>(data_as<uint8_t>() + m_header->start_pos); }
        template<typename T>
        inline T* end_as()                             { return reinterpret_cast<T*>(data_as<uint8_t>() + m_header->start_pos + m_header->size); }
        template<typename T>
        inline T* data_as()                            { return reinterpret_cast<T*>(m_data); }
        template<typename T>
        inline T* at(uint32_t off)                     { return reinterpret_cast<T*>(data_as<uint8_t>() + m_header->start_pos + off); }
        
        inline void* data()                            { return data_as<void>(); }
        inline void* begin()                           { return begin_as<uint8_t>(); }
        inline void* end()                             { return end_as<uint8_t>(); }
        inline void reset()                            { m_header->size = 0; m_header->start_pos = 0; m_header->timestamp = 0; m_header->data_format_hash = data_format_transparent.get_name(); m_header->reference.set_invalid(); }
        inline void set_size(uint32_t size)            { m_header->size = size; }
        inline void set_start_pos(uint32_t pos)        { m_header->start_pos = pos; }
        inline void move_start_pos(uint32_t offset)    { m_header->start_pos += offset; m_header->size -= offset; }
        inline void add_ref()                          { m_header->ref_count.fetch_add(1, std::memory_order_relaxed); if (m_reference) m_reference->add_ref(); }
        inline void release()                          { buffer_manager::get().return_buffer(this); }
        inline void set_timestamp(uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) { m_header->timestamp = ts; }

        void set_referenced_buffer(buffer* buf);
        void set_data_format(const data_format* format);
        bool fill_data(const void* in_data, uint32_t len, uint32_t offset = 0);
        buffer* clone() const;

    protected:

        #pragma pack(push, 1)
        struct header
        {
            std::atomic<uint32_t> ref_count;
            uint32_t              capacity;
            uint32_t              start_pos;
            uint32_t              size;
            string_hash           data_format_hash;
            uint64_t              timestamp;
            buffer_handle         reference;
        };
        #pragma pack(pop)

        buffer();
        ~buffer();

        header*               m_header;                   /**< Pointer to the shared memory header region. */
        buffer*               m_reference;                /**< Pointer to the referenced buffer. */
        void*                 m_data;                     /**< Pointer to the actual memory buffer data. */
        const data_format*    m_data_format;              /**< Optional pointer to a data format describing the contents of this buffer, used for automatic parsing/serialization in the ADAM system. */
        buffer_handle         m_handle;                   /**< The buffer handle. */
        bool                  m_is_resolved;              /**< Indicates whether this buffer has been resolved from a handle. */
    };
}