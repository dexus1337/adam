#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"

namespace adam 
{
    buffer::buffer() 
     :  m_ref_count(nullptr), 
        m_data(nullptr),
        m_capacity(0), 
        m_memory_index(0), 
        m_offset(0) 
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
        return { m_memory_index, m_offset, m_capacity };
    }
}