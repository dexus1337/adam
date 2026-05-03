#include "module/module.hpp"


static adam::modules::can::module_can global_instance = adam::modules::can::module_can();


namespace adam::modules::can
{
    module_can::module_can() : module("can", version)
    {
        // Initialize supported data formats here
    }

    module_can::~module_can() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}