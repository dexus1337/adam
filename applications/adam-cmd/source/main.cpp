#include <adam-sdk.hpp>

#include <csignal>
#include <iostream>
#include <atomic>

#ifdef ADAM_PLATFORM_LINUX
void signal_handler(int signal) 
{
    if (signal == SIGINT) 
    {
    }
}
#endif

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

    if (!lg.log(adam::log::info, "hello from adam-cmd"))
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