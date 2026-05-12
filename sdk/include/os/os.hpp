#pragma once

/**
 * @file    os.hpp
 * @author  dexus1337
 * @brief   Operation System functions
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/sdk-api.hpp"
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
    using thread_id     = uint32_t;
    using process_id    = uint32_t;

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

    inline ADAM_SDK_API process_id get_current_process_id()
    {
        #ifdef      ADAM_PLATFORM_LINUX
        return static_cast<process_id>(syscall(SYS_getpid));
        #elifdef    ADAM_PLATFORM_WINDOWS
        return static_cast<process_id>(GetCurrentProcessId());
        #else
        return 0;
        #endif
    }

    ADAM_SDK_API float get_cpu_usage();
    ADAM_SDK_API void get_ram_usage_mb(float& used_mb, float& available_mb);
}
