#include "module/module.hpp"


static const adam::modules::asterix::module_asterix global_instance = adam::modules::asterix::module_asterix();


namespace adam::modules::asterix
{
    module_asterix::module_asterix() : module("asterix")
    {
        // Initialize supported data formats here
    }

    module_asterix::~module_asterix() 
    {
        // Clean up resources if needed
    }
}

extern "C" const adam::module* ADAM_ASTERIX_API get_adam_module() 
{
    return &global_instance;
}