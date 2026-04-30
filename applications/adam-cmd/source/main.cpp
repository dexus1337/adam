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
        adam::controller::stream_log(adam::log::fatal, "Cannot connect to controller", std::cerr);
        return 1;
    }
    
    adam::controller::stream_log(adam::log::info, "Connected to controller!", std::cout);

    if (!lg.connect())
    {
        adam::controller::stream_log(adam::log::warning, "Failed to connect to the logger.", std::cout);
    }
    else
    {
        adam::controller::stream_log(adam::log::info, "Connected to logger!", std::cout);
    }
    
    if (!lg.log(adam::log::info, "hello from adam-cmd"))
    {
        adam::controller::stream_log(adam::log::warning, "Failed to send log to the logger.", std::cout);
    }
    else
    {
        adam::controller::stream_log(adam::log::info, "Successfully sent a log!", std::cout);
    }
    
    if (!lg.destroy())
    {
        adam::controller::stream_log(adam::log::warning, "Failed to destroy the logger.", std::cout);
        return 1;
    }
    
    if (!cmd.destroy())
    {
        adam::controller::stream_log(adam::log::error, "Failed to destroy the logger.", std::cerr);
    }
    
    adam::controller::stream_log(adam::log::info, "Exiting!", std::cout);

    return 0;
}