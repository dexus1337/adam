#pragma once

/**
 * @file    commander.hpp
 * @author  dexus1337
 * @brief   Defines the ADAM commander which allows to send commands to the commander from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include "controller/controller.hpp"


namespace adam 
{
    class command;
    class response;

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

        bool is_active() const { return m_queue_command.is_active(); }

        /** @brief Establishes a connection to the main commander. */
        bool connect();

        /** @brief Disconnect and free resources. */
        bool destroy();

    protected:

        /** @brief Sends a command. */
        bool send_command(const command& cmd, response* resp = nullptr);

        controller::queue_command m_queue_command;
    };
}