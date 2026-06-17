#include "memory/buffer/buffer.hpp"

#include "data/format.hpp"
#include "module/internals/essential/module-essential.hpp"

namespace adam 
{
    buffer::buffer() 
     :  m_header(nullptr), 
        m_reference(nullptr),
        m_data(nullptr),
        m_data_format(&data_format_transparent),
        m_handle(),
        m_is_resolved(false)
    {
    }

    buffer::~buffer() 
    {
    }

    void buffer::set_data_format(const data_format* format)
    {
        m_data_format = format;
        m_header->data_format_hash = format ? format->get_name().get_hash() : 0;
    }

    void buffer::set_referenced_buffer(buffer* buf)
    {
        m_reference = buf;
        if (buf)
            m_header->reference = buf->get_handle();
        else
            m_header->reference.set_invalid();
    }

    bool buffer::fill_data(const void* in_data, uint32_t len, uint32_t offset)
    {
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
    
}