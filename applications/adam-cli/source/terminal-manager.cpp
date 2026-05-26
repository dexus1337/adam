#include "terminal-manager.hpp"

#ifdef ADAM_PLATFORM_LINUX
#include <unistd.h>
#include <stdio.h>

terminal_manager::terminal_manager() 
{
    tcgetattr(STDIN_FILENO, &oldt);
    struct termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

terminal_manager::~terminal_manager() 
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int terminal_manager::read_char_raw()
{
    return getchar();
}

#elif defined(ADAM_PLATFORM_WINDOWS)
#include <conio.h>

terminal_manager::terminal_manager() 
{
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) 
    {
        GetConsoleMode(hOut, &dwOriginalOutMode);
        SetConsoleMode(hOut, dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

terminal_manager::~terminal_manager() 
{
    if (hOut != INVALID_HANDLE_VALUE) 
    {
        SetConsoleMode(hOut, dwOriginalOutMode);
    }
}

int terminal_manager::read_char_raw()
{
    int ch = _getch();
    if (ch == 0 || ch == 224) { _getch(); return -2; } // Ignore arrow keys / special keys
    return ch;
}
#endif