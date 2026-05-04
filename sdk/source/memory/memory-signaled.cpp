#include "memory/memory-signaled.hpp"

namespace adam 
{
    memory_signaled::memory_signaled(const string_hashed& name) 
     :  memory(name),
        #ifdef ADAM_PLATFORM_LINUX
        m_sem(nullptr)
        #elifdef ADAM_PLATFORM_WINDOWS
        m_handle(nullptr)
        #endif
    {
        #ifdef ADAM_PLATFORM_LINUX
        m_memory_offset = sizeof(sem_t);
        #endif
    }

    memory_signaled::~memory_signaled() { destroy(); }

    bool memory_signaled::create(uint64_t buffer_size) 
    {
        #ifdef ADAM_PLATFORM_LINUX
        uint64_t actual_size = buffer_size + sizeof(sem_t);
        #else
        uint64_t actual_size = buffer_size;
        #endif

        if (!memory::create(actual_size))
            return false;

        #ifdef   ADAM_PLATFORM_LINUX
        m_sem = reinterpret_cast<sem_t*>(m_shared_memory_base);
        if (sem_init(m_sem, 1, 0) == -1) 
        {
            memory::destroy();
            return false;
        }
        #elifdef ADAM_PLATFORM_WINDOWS
        std::string event_name = std::string(get_name().c_str()) + "_signal";
        m_handle = CreateEventA(NULL, FALSE, FALSE, event_name.c_str());
        if (m_handle == NULL)
        {
            memory::destroy();
            return false;
        }
        #endif

        return true;
    }

    bool memory_signaled::open() 
    {
        if (!memory::open())
            return false;

        #ifdef   ADAM_PLATFORM_LINUX
        m_sem = reinterpret_cast<sem_t*>(m_shared_memory_base);
        #elifdef ADAM_PLATFORM_WINDOWS
        std::string event_name = std::string(get_name().c_str()) + "_signal";
        m_handle = OpenEventA(EVENT_ALL_ACCESS, FALSE, event_name.c_str());
        if (m_handle == NULL)
        {
            memory::destroy();
            return false;
        }
        #endif

        return true;
    }

    bool memory_signaled::destroy() 
    {
        if (!m_b_active) 
            return true;

        notify();
        
        bool result = true;

        #ifdef   ADAM_PLATFORM_LINUX
        if (m_sem && m_is_owner)
            result &= (sem_destroy(m_sem) == 0);
        m_sem = nullptr;
        #elifdef ADAM_PLATFORM_WINDOWS
        if (m_handle)
            result &= static_cast<bool>(CloseHandle(m_handle));
        m_handle = nullptr;
        #endif

        result &= memory::destroy();
        return result;
    }

    bool memory_signaled::notify() 
    {
        if (!m_b_active) return false;

        #ifdef   ADAM_PLATFORM_LINUX
        if (!m_sem) return false;
        return sem_post(m_sem) == 0;
        #elifdef ADAM_PLATFORM_WINDOWS
        if (!m_handle) return false;
        return SetEvent(m_handle) != 0;
        #endif
    }

    bool memory_signaled::wait(int32_t timeout_ms) 
    {
        if (!m_b_active) return false;

        #ifdef ADAM_PLATFORM_LINUX
        if (!m_sem) return false;

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
        if (ts.tv_nsec >= 1000000000) 
        {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }

        return sem_timedwait(m_sem, &ts) == 0;

        #elif defined(ADAM_PLATFORM_WINDOWS)
        if (!m_handle) return false;
        DWORD timeout = (timeout_ms < 0) ? INFINITE : static_cast<DWORD>(timeout_ms);
        return WaitForSingleObject(m_handle, timeout) == WAIT_OBJECT_0;
        #endif
    }
}