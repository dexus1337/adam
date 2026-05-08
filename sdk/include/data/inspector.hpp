#pragma once

/**
 * @file    inspector.hpp
 * @author  dexus1337
 * @brief   Defines a class which enables to route data from ports in order to read-only inspect
 * @version 1.0
 * @date    07.05.2026
 */

 
#include "api/api.hpp"
#include "types/queue-shared.hpp"
#include "memory/buffer/buffer.hpp"
#include <thread>
#include <atomic>
#include <functional>

namespace adam 
{
    /**
     * @class   data_inspector
     * @brief   Defines a class which enables to route data from ports in order to read-only inspect
     */
    class ADAM_SDK_API data_inspector
    {
    public:

        /** @brief Constructs a new data processor object. */
        data_inspector();

        /** @brief Destroys the data processor object and cleans up resources. */
        ~data_inspector();

        /** @brief Creates the underlying buffer queue */
        bool create(const string_hashed& port_name);

        /** @brief Opens an existing buffer queue */
        bool open(const string_hashed& port_name);

        /** @brief Data management routine */
        bool handle_data(const buffer* buffer);
        
        /** @brief Starts a background thread that continuously pops buffer handles and resolves them. */
        bool start_inspecting(std::function<void(buffer*)> callback);

        /** @brief Stops the background inspecting thread. */
        void destroy();

    protected:

        static constexpr const char* queue_name_prefix = "adam::data_director_"; /**< The prefix for the name of the buffer queue, followed by the thread id and port id/hash */

        queue_shared<buffer_handle> m_buffer_queue; /**< The queue for incoming buffers */

        void run_inspector(std::function<void(buffer*)> callback);

        std::thread m_inspector_thread;
    };
}