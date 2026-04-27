#include "controller/command/command-queue.hpp"

#include <new> // For placement new

namespace adam 
{
    command_queue::command_queue(const string_hashed& name)
     :  m_shared_memory(name)
    {
    }

    command_queue::~command_queue()
    {
        destroy();
    }

    bool command_queue::create(uint32_t max_commands)
    {
        if (max_commands == 0)
            return false;

        // Calculate total size: queue_header already contains 1 command, so we add space for max_commands - 1
        uint64_t buffer_size = sizeof(queue_header) + sizeof(command) * (max_commands - 1);

        if (!m_shared_memory.create(buffer_size))
            return false;

        // Initialize queue_header and commands in shared memory using placement new
        queue_header* header = reinterpret_cast<queue_header*>(m_shared_memory.get());
        new (&header->head) std::atomic<uint32_t>(0);
        new (&header->tail) std::atomic<uint32_t>(0);
        
        for (uint32_t i = 0; i < max_commands; i++)
            new (&header->commands[i]) command();

        header->max_commands = max_commands;

        return true;
    }

    bool command_queue::open()
    {
        return m_shared_memory.open();
    }

    bool command_queue::destroy()
    {
        return m_shared_memory.destroy();
    }

    bool command_queue::push(const command& cmd)
    {
        auto* header = this->header();
        
        uint32_t current_head = header->head.load(std::memory_order_relaxed);
        uint32_t next_head = (current_head + 1) % header->max_commands;

        if (next_head == header->tail.load(std::memory_order_acquire))
            return false; // Queue full

        header->commands[current_head] = cmd;
        header->head.store(next_head, std::memory_order_release);
        
        return m_shared_memory.signal().notify();
    }

    bool command_queue::pop(command& out_cmd, int32_t timeout)
    {
        // Wait for a signal that data is available
        if (!m_shared_memory.signal().wait(timeout))
            return false; // Timeout

        auto* header = this->header();
        uint32_t current_tail = header->tail.load(std::memory_order_relaxed);
        
        if (current_tail == header->head.load(std::memory_order_acquire))
            return false; // Spurious wake-up / Empty

        out_cmd = header->commands[current_tail];
        header->tail.store((current_tail + 1) % header->max_commands, std::memory_order_release);
        
        return true;
    }
}
