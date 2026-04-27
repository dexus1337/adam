#pragma once

/**
 * @file        commander.hpp
 * @author      dexus1337
 * @brief       Defines the ADAM commander which allows to send commands to the controller from external processes
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include "memory/shared/memory-shared.hpp"
#include "queue/queue-shared.hpp"


namespace adam 
{
    class command;

    /**
     * @class commander
     * @brief Defines the ADAM commander which allows to send commands to the controller from external processes
     */
    class ADAM_SDK_API commander
    {
    public:

        /** @brief Constructs a new controller object.*/
        commander();

        /** @brief Destroys the controller object and cleans up resources.*/
        ~commander();

        /** @brief Establishes a connection to the main controller. */
        bool connect();

        /** @brief Disconnect and free resources. */
        bool destroy();

    protected:

        /** @brief Sends a command. */
        bool send_command(const command& cmd);

        queue_shared<command> m_cmd_queue;
    };
}