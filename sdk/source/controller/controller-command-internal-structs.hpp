#pragma once

/**
 * @file        controller-command-internal-structs.hpp
 * @author      dexus1337
 * @brief       Defines internal structures and types used for command management within the ADAM controller system, facilitating communication and command execution across processes and modules.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include "memory/shared/memory-shared.hpp"

#include <atomic>
#include <cstddef>


namespace adam 
{
    struct command
    {
        command_type type;   /**< The type of the command, used to identify the action to be performed. */
        uint64_t secret;     /**< A secret value, used for authentication or other purposes. */
    };

    struct command_queue
    {
        static constexpr size_t queue_size = 1024;  /**< The maximum number of commands that can be queued in the command queue, used for interprocess communication between the main controller and external processes. */
    
        std::atomic<uint32_t> head;                 /**< The index of the head of the command queue, used for tracking where new commands should be added. */
        std::atomic<uint32_t> tail;                 /**< The index of the tail of the command queue, used for tracking where commands should be read from. */
    
        command commands[queue_size];               /**< The array of commands in the command queue, used for storing commands sent from external processes to the main controller. */
    };
    
}