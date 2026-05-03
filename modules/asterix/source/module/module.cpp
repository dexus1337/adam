#include "module/module.hpp"


static adam::modules::asterix::module_asterix global_instance = adam::modules::asterix::module_asterix();


namespace adam::modules::asterix
{
    module_asterix::module_asterix() : module("asterix", version, adam::make_version(2,0,0))
    {
        // Initialize supported data formats here
    }

    module_asterix::~module_asterix() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}
