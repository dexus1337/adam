#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "data/format.hpp"

namespace adam 
{
    buffer::buffer() 
     :  m_header(nullptr), 
        m_data(nullptr),
        m_data_format(&data_format_transparent),
        m_handle(),
        m_is_resolved(false)
    {
    }

    buffer::~buffer() 
    {
    }

    void buffer::release() 
    {
        if (m_header && m_header->ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            buffer_manager::get().return_buffer(this);
        }
    }

    buffer_handle buffer::get_handle() const 
    {
        return m_handle;
    }

    bool buffer::fill_data(const void* in_data, uint32_t len, uint32_t offset)
    {
        if (!m_header)
        {
            return false;
        }

        // Prevent out-of-bounds writing
        if (offset + len > m_header->capacity)
        {
            return false;
        }

        // Copy the data into the buffer at the specified offset
        std::memcpy(static_cast<uint8_t*>(m_data) + offset, in_data, len);
        
        // Extend the size tracker if we wrote past the previous end
        if (offset + len > m_header->size)
        {
            m_header->size = offset + len;
        }

        return true;
    }

    void buffer::set_data_format(const data_format* format)
    {
        m_data_format = format;
        if (m_header)
        {
            m_header->data_format_hash = format ? format->get_name().get_hash() : 0;
        }
    }
}