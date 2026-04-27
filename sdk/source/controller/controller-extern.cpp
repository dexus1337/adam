#include "controller/controller-extern.hpp"


#include "controller/controller.hpp"


namespace adam 
{
    controller_extern::controller_extern() : m_shared_memory(string_hashed(controller::cmd_memory_name)) {}

    controller_extern::~controller_extern() {}

    bool controller_extern::connect(const string_hashed& secret) 
    {
        if (!m_shared_memory.open())
            return false;

        

        return true; // Successfully connected
    }

    void controller_extern::disconnect() 
    {
        m_shared_memory.destroy();
    }

    bool controller_extern::send_command(const command* cmd) 
    {
        /*command_queue_header* cmd_queue = reinterpret_cast<command_queue_header*>(m_shared_memory.get());

        if (!cmd_queue)
            return false;

        uint32_t h = cmd_queue->head.load(std::memory_order_relaxed);
        uint32_t t = cmd_queue->tail.load(std::memory_order_acquire);

        uint32_t next_h = (h + 1) % command_queue_header::queue_size;

        if (next_h == t) {
            return false; // Queue is full!
        }

        cmd_queue->commands[h] = *cmd;
        cmd_queue->head.store(next_h, std::memory_order_release);
        
        return true;*/

        return false;
    }
}