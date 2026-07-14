#include <adam-sdk.hpp>

#include <csignal>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
DWORD orig_console_mode;
void disable_echo() 
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) return;
    GetConsoleMode(hStdin, &orig_console_mode);
    SetConsoleMode(hStdin, orig_console_mode & ~ENABLE_ECHO_INPUT);
}
void restore_echo() 
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) return;
    SetConsoleMode(hStdin, orig_console_mode);
}
#else
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;
void disable_echo() 
{
    if (!isatty(STDIN_FILENO)) return;
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
void restore_echo() 
{
    if (!isatty(STDIN_FILENO)) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
#endif

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
    disable_echo();

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    adam::controller::cleanup_zombie_shared_memory();

    auto& controller = adam::controller::get();

    if (!controller.run(true))
    {
        controller.log(adam::log::error, "CRITICAL: failed to start adam!");
        restore_echo();
        return 1;
    }

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

    restore_echo();
    return 0;
}
