#include <adam-sdk.hpp>


int main() 
{
    adam::controller controller = adam::controller();

    controller.scan_for_modules("./modules/");

    for (const auto& [name, mod] : controller.get_modules())
    {
        printf("Loaded module: %s version %x (%s)\n", name.c_str(), mod->get_version(), mod->get_filepath().c_str());
    }
    
    
    return 0;
}