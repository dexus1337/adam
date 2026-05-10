#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "data/format.hpp"

namespace adam 
{
    buffer::buffer() 
     :  m_ref_count(nullptr), 
        m_data(nullptr),
        m_data_format(&data_format_transparent),
        m_capacity(0),
        m_size(0),
        m_memory_index(0), 
        m_thread_id(0),
        m_offset(0),
        m_is_resolved(false)
    {
    }

    buffer::~buffer() 
    {
    }

    void buffer::release() 
    {
        if (m_ref_count && m_ref_count->fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            buffer_manager::get().return_buffer(this);
        }
    }

    buffer_handle buffer::get_handle() const 
    {
        // Assuming string_hashed exposes .get_hash() or similar to retrieve the uint64_t hash.
        uint64_t fmt_hash = m_data_format ? m_data_format->get_name().get_hash() : 0;
        return { m_memory_index, m_thread_id, m_offset, m_capacity, m_size, fmt_hash };
    }

    bool buffer::fill_data(const void* in_data, uint32_t len, uint32_t offset)
    {
        // Prevent out-of-bounds writing
        if (offset + len > m_capacity)
        {
            return false;
        }

        // Copy the data into the buffer at the specified offset
        std::memcpy(static_cast<uint8_t*>(m_data) + offset, in_data, len);
        
        // Extend the size tracker if we wrote past the previous end
        if (offset + len > m_size)
        {
            m_size = offset + len;
        }

        return true;
    }
}