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
    adam::commander cmd     = adam::commander();
    adam::logger lg         = adam::logger();
    adam::logger_sink lgsnk = adam::logger_sink();

    if (!cmd.connect())
    {
        adam::stream_log(adam::log::fatal, "Cannot connect to controller", std::cerr);
        return 1;
    }
    
    adam::stream_log(adam::log::info, "Connected to controller!", std::cout);

    if (!lg.connect())
    {
        adam::stream_log(adam::log::warning, "Failed to connect to the logger.", std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, "Connected to logger!", std::cout);
    }
    
    if (!lg.log(adam::log::info, "hello from adam-cmd"))
    {
        adam::stream_log(adam::log::warning, "Failed to send log to the logger.", std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, "Successfully sent a log!", std::cout);
    }

    std::thread test_heartbeat([&]()
    {
        while (lg.is_active())
        {
            lg.log(adam::log::info, "adam-cmd is also alife!");
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    if (!lgsnk.connect())
    {
        adam::stream_log(adam::log::warning, "Failed to connect to the logger sink.", std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, "Connected to logger sink!", std::cout);
    }
    
    std::thread logger_sink_watcher([&]() 
    {
        while (lgsnk.queue().is_active())
        {
            adam::log cur_log;

            if (!lgsnk.queue().pop(cur_log, 100))
                continue;

            adam::stream_log(cur_log, std::cout);
        }
    });

    getchar();

    lgsnk.queue().disable();

    logger_sink_watcher.join();

    if (!lgsnk.destroy())
    {
        adam::stream_log(adam::log::warning, "Failed to destroy the logger sink.", std::cout);
    }

    if (!lg.destroy())
    {
        adam::stream_log(adam::log::warning, "Failed to destroy the logger.", std::cout);
    }
    
    if (!cmd.destroy())
    {
        adam::stream_log(adam::log::error, "Failed to destroy the logger.", std::cerr);
    }

    test_heartbeat.join();
    
    adam::stream_log(adam::log::info, "Exiting!", std::cout);

    return 0;
}
