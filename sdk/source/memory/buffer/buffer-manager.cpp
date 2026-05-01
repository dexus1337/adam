#include "memory/buffer/buffer-manager.hpp"

namespace adam 
{
    buffer_manager& buffer_manager::get() 
    {
        static buffer_manager instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

    bool buffer_manager::initialize() 
    {
        return true;
    }

    bool buffer_manager::destroy() 
    {
        return true;
    }

    buffer_manager::buffer_manager() {}

    buffer_manager::~buffer_manager() {}
}