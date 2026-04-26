#include <adam-sdk.hpp>


int main() 
{
    adam::controller controller = adam::controller();

    controller.scan_for_modules("./modules/");

    for (const auto& [name, path] : controller.get_available_modules())
    {
        printf("Available module: %s (%s)\n", name.c_str(), path.c_str());
    }
    
    for (const auto& [name, path] : controller.get_available_modules())
    {
        if (controller.load_module(name))
        {
            printf("Successfully loaded module: %s -> %llx\n", name.c_str(), controller.get_loaded_module(name)->get_module_handle());
        }
        else
        {
            printf("Failed to load module: %s\n", name.c_str());
        }
    }
    
    
    return 0;
}