#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "data/format.hpp"

namespace adam 
{
    buffer::buffer() 
     :  m_ref_count(nullptr), 
        m_data(nullptr),
        m_capacity(0), 
        m_memory_index(0), 
        m_thread_id(0),
        m_offset(0),
        m_is_resolved(false),
        m_data_format(&data_format_transparent)
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
        return { m_memory_index, m_thread_id, m_offset, m_capacity, fmt_hash };
    }
}