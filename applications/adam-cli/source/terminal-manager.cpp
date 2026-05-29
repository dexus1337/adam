#include "terminal-manager.hpp"

#ifdef ADAM_PLATFORM_LINUX
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>

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
    unsigned char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1) return EOF;
    if (ch == 27)
    {
        fd_set set;
        struct timeval tv;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        tv.tv_sec = 0;
        tv.tv_usec = 10000; // 10ms
        
        if (select(STDIN_FILENO + 1, &set, NULL, NULL, &tv) > 0)
        {
            unsigned char ch2;
            if (read(STDIN_FILENO, &ch2, 1) == 1 && ch2 == '[')
            {
                FD_ZERO(&set);
                FD_SET(STDIN_FILENO, &set);
                tv.tv_sec = 0;
                tv.tv_usec = 10000; // 10ms
                if (select(STDIN_FILENO + 1, &set, NULL, NULL, &tv) > 0)
                {
                    unsigned char ch3;
                    if (read(STDIN_FILENO, &ch3, 1) == 1)
                    {
                        if (ch3 == 'A') return key_up;
                        if (ch3 == 'B') return key_down;
                        if (ch3 == 'C') return key_right;
                        if (ch3 == 'D') return key_left;
                    }
                    return -2;
                }
            }
        }
        return -2; // Ignore unhandled escape sequences
    }
    return ch;
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
    if (ch == 0 || ch == 224) 
    { 
        int ch2 = _getch(); 
        if (ch2 == 72) return key_up;
        if (ch2 == 80) return key_down;
        if (ch2 == 77) return key_right;
        if (ch2 == 75) return key_left;
        return -2; 
    } // Ignore other arrow keys / special keys
    return ch;
}
#endif