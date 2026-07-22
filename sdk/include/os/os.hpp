#pragma once

/**
 * @file    os.hpp
 * @author  dexus1337
 * @brief   Operation System functions
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api-sdk.hpp"
#include <cstdint>
#include <atomic>

#ifdef ADAM_PLATFORM_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef ADAM_CPU_X64
#include <immintrin.h>
#endif

namespace adam::cpu
{
    inline void pause()
    {
        #ifdef ADAM_CPU_X64
        _mm_pause();
        #endif
    }
}

namespace adam::spinlock
{
    inline void acquire(std::atomic_flag& lock)
    {
        while (lock.test_and_set(std::memory_order_acquire))
        {
            while (lock.test(std::memory_order_relaxed)) { cpu::pause(); }
        }
    }

    inline void release(std::atomic_flag& lock)
    {
        lock.clear(std::memory_order_release);
    }

    struct guard
    {
        std::atomic_flag& m_lock;
        guard(std::atomic_flag& lock) : m_lock(lock) { acquire(m_lock); }
        ~guard() { release(m_lock); }
    };
}

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

    extern ADAM_SDK_API float   get_cpu_usage();
    extern ADAM_SDK_API void    get_ram_usage_mb(float& used_mb, float& available_mb);

    extern ADAM_SDK_API void*   load_library(const char* path);
    extern ADAM_SDK_API bool    unload_library(void* handle);
    extern ADAM_SDK_API void*   get_library_symbol(void* handle, const char* symbol);
}
