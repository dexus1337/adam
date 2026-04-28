#include <adam-sdk.hpp>
#include <iostream>


int main() 
{
    adam::commander cmd = adam::commander();
    adam::logger lg     = adam::logger();

    if (!cmd.connect())
    {
        std::cerr << "Failed to connect to the controller." << std::endl;
        return 1;
    }
    
    std::cout << "Connected to controller!" << std::endl;

    if (!lg.connect())
    {
        std::cerr << "Failed to connect to the logger." << std::endl;
        return 1;
    }
    
    std::cout << "Connected to logger!" << std::endl;

    if (!lg.log("hello from adam-cmd", adam::log::info))
    {
        std::cerr << "Failed to send log to the logger." << std::endl;
    }
    
    std::cout << "Successfully sent a log!" << std::endl;

    if (!cmd.destroy())
    {
        std::cerr << "Failed to destroy to the controller." << std::endl;
        return 1;
    }
    
    std::cout << "Commander destroyed. Done!" << std::endl;
    
    if (!lg.destroy())
    {
        std::cerr << "Failed to destroy to the logger." << std::endl;
        return 1;
    }
    
    std::cout << "Commander destroyed. Done!" << std::endl;
    
    return 0;
}