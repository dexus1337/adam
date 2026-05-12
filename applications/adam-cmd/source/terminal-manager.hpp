#pragma once

#ifdef ADAM_PLATFORM_LINUX
#include <termios.h>
#elif defined(ADAM_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

class terminal_manager 
{
#ifdef ADAM_PLATFORM_LINUX
    struct termios oldt;
#elif defined(ADAM_PLATFORM_WINDOWS)
    HANDLE hOut;
    DWORD dwOriginalOutMode = 0;
#endif

public:
    terminal_manager();
    ~terminal_manager();

    static int read_char_raw();
};