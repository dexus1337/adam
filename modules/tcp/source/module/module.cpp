#include "module/module.hpp"


static adam::modules::tcp::module_tcp global_instance = adam::modules::tcp::module_tcp();


namespace adam::modules::tcp
{
    module_tcp::module_tcp() : module("tcp", version)
    {
        // Initialize supported data formats here
    }

    module_tcp::~module_tcp() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}