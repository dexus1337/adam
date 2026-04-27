#include "memory/shared/memory-shared-signal.hpp"

#include "memory/shared/memory-shared.hpp"

namespace adam 
{
    memory_shared_signal::memory_shared_signal(memory_shared* shared_memory) 
     :  m_shared_memory(shared_memory),
        #ifdef ADAM_PLATFORM_LINUX
        m_sem(nullptr)
        #elifdef ADAM_PLATFORM_WINDOWS
        m_handle(nullptr)
        #endif
    {

    }

    memory_shared_signal::~memory_shared_signal() {}

    bool memory_shared_signal::create() 
    {
        // OS-specific initialization of the signal backend
        #ifdef   ADAM_PLATFORM_LINUX
        if (sem_init(m_shared_memory->get_signal_semaphore(), 1, 0) == -1) // Initialize semaphore at the start of the shared memory, second argument 1 is for interprocess
            return false;

        m_sem = m_shared_memory->get_signal_semaphore();

        return true;
        #elifdef ADAM_PLATFORM_WINDOWS
        m_handle = CreateEventA(NULL, FALSE, FALSE, m_shared_memory->get_name().c_str());
        return m_handle != NULL;
        #else
        return false; // Unsupported platform
        #endif
    }

    bool memory_shared_signal::open() 
    {
        // OS-specific initialization of the signal backend
        #ifdef   ADAM_PLATFORM_LINUX
        m_sem = m_shared_memory->get_signal_semaphore();
        return true;
        #elifdef ADAM_PLATFORM_WINDOWS
        m_handle = OpenEventA(EVENT_ALL_ACCESS, FALSE, m_shared_memory->get_name().c_str());
        return m_handle != NULL;
        #else
        return false; // Unsupported platform
        #endif
    }

    bool memory_shared_signal::destroy() 
    {
        #ifdef   ADAM_PLATFORM_LINUX
        if (!m_sem)
            return true;
        #elifdef ADAM_PLATFORM_WINDOWS
        if (!m_handle)
            return true;
        #endif
        
        notify();

        #ifdef   ADAM_PLATFORM_LINUX
        if (sem_destroy(m_sem) != 0)
            return false;
        m_sem = nullptr;
        #elifdef ADAM_PLATFORM_WINDOWS
        if (!CloseHandle(m_handle))
            return false;
        m_handle = nullptr;
        #endif
        return true;
    }

    bool memory_shared_signal::notify() 
    {
        // OS-specific signal activation
        #ifdef   ADAM_PLATFORM_LINUX
        return sem_post(m_sem) == 0;
        #elifdef ADAM_PLATFORM_WINDOWS
        return SetEvent(m_handle) != 0;
        #endif
    }

    bool memory_shared_signal::wait(int32_t timeout_ms) 
    {
        #ifdef ADAM_PLATFORM_LINUX
        if (timeout_ms < 0)
            return sem_wait(m_sem) == 0;

        // sem_timedwait requires an absolute end time (CLOCK_REALTIME)
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return false;

        // Add the timeout duration to the current time
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        // Handle nanosecond overflow
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }

        return sem_timedwait(m_sem, &ts) == 0;

        #elif defined(ADAM_PLATFORM_WINDOWS)
        DWORD timeout = (timeout_ms < 0) ? INFINITE : static_cast<DWORD>(timeout_ms);
        return WaitForSingleObject(m_handle, timeout) == WAIT_OBJECT_0;
        #endif
    }
}