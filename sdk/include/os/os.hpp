#pragma once

/**
 * @file    os.hpp
 * @author  dexus1337
 * @brief   Operation System functions
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"
#include <cstdint>

#ifdef      ADAM_PLATFORM_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif


namespace adam::os
{
    using thread_id = uint64_t;

    inline ADAM_SDK_API thread_id get_current_thread_id()
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
