#include "module/module.hpp"


static adam::modules::network::module_network global_instance = adam::modules::network::module_network();


namespace adam::modules::network
{
    module_network::module_network() : module("network", version)
    {
        // Initialize supported data formats here
    }

    module_network::~module_network() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}