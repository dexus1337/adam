#pragma once

/**
 * @file    commander.hpp
 * @author  dexus1337
 * @brief   Defines the ADAM commander which allows to send commands to the commander from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include "controller/controller.hpp"
#include "data/inspector.hpp"

#include <unordered_map>
#include <atomic>
#include <thread>
#include <functional>


namespace adam 
{
    class command;
    class response;
    class event;

    /**
     * @class commander
     * @brief Defines the ADAM commander which allows to send commands to the controller from external processes
     */
    class ADAM_SDK_API commander
    {
    public:

        /** @brief Constructs a new commander object.*/
        commander();

        /** @brief Destroys the commander object and cleans up resources.*/
        ~commander();

        bool is_active() const { return m_queue_command.is_active() && m_queue_event.is_active(); }

        /** @brief Establishes a connection to the main commander. */
        bool connect();

        /** @brief Disconnect and free resources. */
        bool destroy();

        /** @brief Requests the creation of a data inspector on a specific port. */
        response_status request_inspector_create(const string_hashed& port_name, std::function<void(buffer*)> callback, data_inspector*& out_inspector);

        /** @brief Requests the destruction of a data inspector on a specific port. */
        response_status request_inspector_destroy(data_inspector* inspector);

        /** @brief Sets a callback to be invoked when an event is received from the controller. */
        void set_event_callback(std::function<void(const event&)> callback) { m_event_callback = std::move(callback); }

    protected:

        /** @brief Sends a command. Can have multiple (extended) responses. */
        response_status send_command(const command& cmd, response** resp = nullptr);

        void run_event_loop();

        controller::queue_command m_queue_command;
        controller::queue_event   m_queue_event;
        
        std::thread               m_event_thread;
        std::function<void(const event&)> m_event_callback;

        std::unordered_map<string_hashed::hash_datatype, data_inspector*> m_inspectors;
    };
}