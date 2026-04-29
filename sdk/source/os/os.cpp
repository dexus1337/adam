#include "os/os.hpp"

#ifdef      ADAM_PLATFORM_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam::os
{
    inline thread_id get_current_thread_id()
    {
        #ifdef      ADAM_PLATFORM_LINUX
        return static_cast<thread_id>(syscall(SYS_gettid));
        #elifdef    ADAM_PLATFORM_WINDOWS
        return static_cast<thread_id>(GetCurrentThreadId());
        #else
        return 0;
        #endif
    }
}
