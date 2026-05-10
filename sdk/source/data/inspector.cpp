#include "data/inspector.hpp"
#include "memory/buffer/buffer-manager.hpp"

namespace adam 
{
    data_inspector::data_inspector()
     :  m_buffer_queue(),
        m_inspector_thread()
    {
    }

    data_inspector::~data_inspector()
    {
        destroy();
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
    bool data_inspector::open(const string_hashed& port_name, os::thread_id tid)
    {
        m_buffer_queue.set_name(string_hashed(queue_name_prefix + std::to_string(tid) + "_" + std::to_string(port_name.get_hash())));

        if (!m_buffer_queue.open())
            return false;

        return true;
    }

    /** @brief Data management routine */
    bool data_inspector::handle_data(const buffer* buffer)
    {
        return m_buffer_queue.push(buffer->get_handle());
    }

    bool data_inspector::start_inspecting(std::function<void(buffer*)> callback)
    {
        if (m_inspector_thread.joinable())
            return false;

        m_inspector_thread = std::thread(&data_inspector::run_inspector, this, callback);

        return true;
    }

    void data_inspector::destroy()
    {
        m_buffer_queue.disable(); // Wake up the blocking pop() if the thread is waiting

        if (m_inspector_thread.joinable())
            m_inspector_thread.join();
            
        m_buffer_queue.destroy();
    }

    void data_inspector::run_inspector(std::function<void(buffer*)> callback)
    {
        while (m_buffer_queue.is_active())
        {
            buffer_handle handle;

            // Try to pop a buffer handle from the queue with a timeout
            if (!m_buffer_queue.pop(handle, 100))
                continue;

            // Use the buffer_manager to resolve the buffer_handle into a buffer pointer
            buffer* resolved_buffer = buffer_manager::get().resolve_handle(handle);

            if (resolved_buffer && callback)
                callback(resolved_buffer);
        }
    }
}