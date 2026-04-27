#include "memory/manager/memory-manager.hpp"

namespace adam 
{
    memory_manager& memory_manager::get() 
    {
        static memory_manager instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

    bool memory_manager::initialize(uint32_t buffer_size) 
    {
        // Initialize shared memory region for memory buffers here
        m_shared_memory_base = nullptr; // Placeholder for actual shared memory initialization
        m_shared_memory_size = buffer_size; // Placeholder for actual size of the shared memory region

        return true; // Return true if initialization is successful, false otherwise
    }

    void memory_manager::destroy() 
    {
        // Clean up shared memory resources here
        m_shared_memory_base = nullptr;
        m_shared_memory_size = 0;
    }

    memory_manager::memory_manager() {}

    memory_manager::~memory_manager() {}
}