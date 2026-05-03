#include <adam-sdk.hpp>

#include <csignal>
#include <iostream>
#include <atomic>

adam::controller* controller;

void signal_handler(int signal) 
{
    if (signal == SIGINT) 
    {
        controller->log(adam::log::info, std::format("sigint recieved with signal {:d}", signal));

        try 
        {
            if (controller)
                controller->destroy();
        } 
        catch (const std::system_error& e) 
        {
            controller->log(adam::log::error, std::format("os description \"{:s}\" (code {:d})", e.what(), e.code().value()));
        }

        controller->log(adam::log::info, std::format("exiting...", signal));
    }
}

int main() 
{
    std::signal(SIGINT, signal_handler);

    controller = new adam::controller();

    controller->scan_for_modules("./modules/");

    for (const auto& [name, path] : controller->get_available_modules())
    {
        auto ver = adam::decode_version(path.first);

        controller->log(adam::log::info, std::format("Available module: {} ver {:d}.{:d}.{:d} -> ({})", 
                                  name.c_str(), ver.major, ver.minor, ver.patch, path.second.c_str()));
    }
    
    for (const auto& [name, path] : controller->get_available_modules())
    {
        if (controller->load_module(name))
        {
            controller->log(adam::log::info, std::format("loaded module \"{}\" at 0x{:x}", name.c_str(), controller->get_loaded_module(name)->get_module_handle()));
        }
        else
        {
            controller->log(adam::log::error, std::format("failed to load module \"{}\"", name.c_str()));
        }
    }

    controller->log(adam::log::info, "adam up and running!");

    std::thread test_heartbeat([&]()
    {
        while (controller->is_active())
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            controller->log(adam::log::info, "adam is alife!");
        }
    });

    controller->run();

    return 0;
}
