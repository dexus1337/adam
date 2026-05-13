#include <adam-sdk.hpp>

#include <csignal>
#include <iostream>
#include <atomic>

void signal_handler(int signal) 
{
    if (signal == SIGINT) 
    {
        adam::controller::get().log(adam::log::info, std::format("sigint recieved with signal {:d}", signal));

        try 
        {
            adam::controller::get().destroy();
        } 
        catch (const std::system_error& e) 
        {
            adam::controller::get().log(adam::log::error, std::format("os description \"{:s}\" (code {:d})", e.what(), e.code().value()));
        }

        adam::controller::get().log(adam::log::info, std::format("exiting...", signal));
    }
}

int main() 
{
    std::signal(SIGINT, signal_handler);

    auto& controller = adam::controller::get();

    controller.modules().scan_for_modules("./modules/");

    for (const auto& [name, path] : controller.modules().get_available_modules())
    {
        controller.modules().load_module(name);
    }

    if (controller.run(true))
    {
        controller.log(adam::log::info, "adam stared succesfully!");
    }
    else
    {
        controller.log(adam::log::error, "failed to start adam!");
    }

    std::thread test_heartbeat([&]()
    {
        while (controller.is_active())
        {
            controller.log(adam::log::info, "adam is alife!");
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    getchar();

    controller.destroy();

    test_heartbeat.join();

    return 0;
}
