#include "module/module.hpp"


static adam::modules::recrep::module_recrep global_instance = adam::modules::recrep::module_recrep();


namespace adam::modules::recrep
{
    module_recrep::module_recrep() : module("recrep", version)
    {
        // Initialize supported data formats here
    }

    module_recrep::~module_recrep() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* ADAM_RECREP_API get_adam_module() 
{
    return &global_instance;
}