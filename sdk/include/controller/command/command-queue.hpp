#pragma once

/**
 * @file        command-queue.hpp
 * @author      dexus1337
 * @brief       Interprocess command queue based on shared_memory
 * @version     1.0
 * @date        27.04.2026
 */

 
#include "api/api.hpp"

#include <atomic>
#include <cstddef>

#include "string/string-hashed.hpp"
#include "controller/command/command.hpp"
#include "memory/shared/memory-shared.hpp"

namespace adam 
{
    /**
     * @class command_queue
     * @brief Defines a command for the controller
     */
    class ADAM_SDK_API command_queue
    {
    public:

        /** @brief Constructs a new command_queue object.*/
        command_queue(const string_hashed& name);

        /** @brief Destroys the command_queue object and cleans up resources.*/
        ~command_queue();

        uint32_t get_max_command_count() const { return m_shared_memory.get() ? this->get_header()->max_commands : 0; }

        /** @brief Creates the command queue and the underlying shared_memory for managing a max amount of commands given */
        bool create(uint32_t max_commands);

        /** @brief Opens an existing command queue */
        bool open();

        /** @brief Destroys the queue and free all resources */
        bool destroy();

        /** @brief Pushes a command in the list and notifies the signal */
        bool push(const command& cmd);

        /** @brief Pushes a command in the list and notifies the signal */
        bool pop(command& out_cmd, int32_t timeout = -1);

        /** @brief Pushes a command in the list and notifies the signal. Compatible with std::chrono::duration */
        template <typename rep, typename period>
        bool pop(command& out_cmd, std::chrono::duration<rep, period> timeout = std::chrono::duration<rep, period>::max()) { return pop(out_cmd, timeout == std::chrono::duration<rep, period>::max() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()); }

    protected:

        struct queue_header
        {
            uint32_t max_commands;
            std::atomic<uint32_t> head; /**< The index of the head of the command queue, used for tracking where new commands should be added. */
            std::atomic<uint32_t> tail; /**< The index of the tail of the command queue, used for tracking where commands should be read from. */
        
            command commands[1];
        };

        /** @brief Accessor for header from inside the shared memory */
        queue_header* header() { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        /** @brief Const Accessor for header from inside the shared memory */
        const queue_header* get_header() const { return reinterpret_cast<queue_header*>(m_shared_memory.get()); }

        memory_shared m_shared_memory;
    };
}