#include <adam-sdk.hpp>

#include <csignal>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> running{true};

void signal_handler(int signal) 
{
    if (signal == SIGINT || signal == SIGTERM) 
    {
        running = false;
    }
}

int main() 
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    adam::controller::cleanup_zombie_shared_memory();

    auto& controller = adam::controller::get();

    if (!controller.run(true))
    {
        controller.log(adam::log::error, "CRITICAL: failed to start adam!");
        return 1;
    }

    std::thread([]() 
    {
        if (getchar() != EOF) 
            running = false;
    }).detach();

    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    controller.log(adam::log::info, "exiting...");

    try 
    {
        controller.destroy();
    } 
    catch (const std::system_error& e) 
    {
        controller.log(adam::log::error, std::format("os description \"{:s}\" (code {:d})", e.what(), e.code().value()));
    }

    return 0;
}
