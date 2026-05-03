#include "module/module.hpp"


static adam::modules::udp::module_udp global_instance = adam::modules::udp::module_udp();


namespace adam::modules::udp
{
    module_udp::module_udp() : module("udp", version)
    {
        // Initialize supported data formats here
    }

    module_udp::~module_udp() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}