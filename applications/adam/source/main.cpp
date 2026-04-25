#include <adam-sdk.hpp>


int main() 
{
    adam::controller controller = adam::controller();

    controller.scan_for_modules("./modules/");
    
    return 0;
}