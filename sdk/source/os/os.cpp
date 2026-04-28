#include "os/os.hpp"

#ifdef      ADAM_PLATFORM_LINUX
#include <unistd.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam::os
{
    thread_id get_current_thread_id()
    {
        #ifdef      ADAM_PLATFORM_LINUX
        return static_cast<thread_id>(gettid());
        #elifdef    ADAM_PLATFORM_WINDOWS
        return static_cast<thread_id>(GetCurrentThreadId());
        #else
        return 0;
        #endif
    }
}