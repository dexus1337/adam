#include <adam-sdk.hpp>

#include <csignal>

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

    adam::controller::cleanup_zombie_shared_memory();

    auto& controller = adam::controller::get();

    if (!controller.run(true))
    {
        controller.log(adam::log::error, "CRITICAL: failed to start adam!");
        return 1;
    }

    getchar();

    controller.destroy();

    return 0;
}
