#pragma once

/**
 * @file        controller-extern.hpp
 * @author      dexus1337
 * @brief       Defines the extern ADAM controller class which manages enables other processes to interact with the main controller, providing an interface for interprocess communication and command execution.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include "memory/shared/memory-shared.hpp"


namespace adam 
{
    class command;

    /**
     * @class controller_extern
     * @brief The extern controller class for the ADAM system, responsible for enabling other processes to interact with the main controller, providing an interface for interprocess communication and command execution.
     */
    class ADAM_SDK_API controller_extern
    {
    public:

        /**
         * @brief Constructs a new controller object.
         */
        controller_extern();

        /**
         * @brief Destroys the controller object and cleans up resources.
         */
        ~controller_extern();

        /** @brief Establishes a connection to the main controller. */
        bool connect(const string_hashed& secret);

        /** @brief Disconnects from the main controller. */
        void disconnect();

        // COMMAND MANAGEMENT

        // MODULE MANAGEMENT

    protected:

        /** @brief Sends a command to the main controller. */
        bool send_command(const command* cmd);

        memory_shared m_shared_memory;   /**< Shared memory instance used for communication with the main controller. */

    };
}