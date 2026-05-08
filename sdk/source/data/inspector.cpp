#include "data/inspector.hpp"

namespace adam 
{
    data_inspector::data_inspector()
     :  m_buffer_queue()
    {
    }

    data_inspector::~data_inspector()
    {
        // Clean up resources if necessary (currently none)
    };

    /** @brief Creates a new data director with the given port name and max items for the buffer queue. */
    bool data_inspector::create(const string_hashed& port_name)
    {
        m_buffer_queue.set_name(string_hashed(queue_name_prefix + std::to_string(os::get_current_thread_id()) + "_" + std::to_string(port_name.get_hash())));

        if (!m_buffer_queue.create(0x1000))
            return false;

        return true;
    }

    /** @brief Opens an existing buffer queue */
    bool data_inspector::open(const string_hashed& port_name)
    {
        m_buffer_queue.set_name(string_hashed(queue_name_prefix + std::to_string(os::get_current_thread_id()) + "_" + std::to_string(port_name.get_hash())));

        if (!m_buffer_queue.open())
            return false;

        return true;
    }

    /** @brief Data management routine */
    bool data_inspector::handle_data(const buffer* buffer)
    {
        return m_buffer_queue.push(buffer->get_handle());
    }
        
}