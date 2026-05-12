#include "os/os.hpp"

#ifdef      ADAM_PLATFORM_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#elifdef    ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <chrono>
#include <fstream>
#include <sstream>

namespace adam::os
{
    float get_cpu_usage()
    {
        static float last_cpu_usage = 0.0f;
        static auto last_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() < 500)
            return last_cpu_usage;
            
        #ifdef ADAM_PLATFORM_WINDOWS
        static FILETIME prev_sys_idle, prev_sys_kernel, prev_sys_user;
        static bool first_run = true;
        
        FILETIME sys_idle, sys_kernel, sys_user;
        if (!GetSystemTimes(&sys_idle, &sys_kernel, &sys_user))
            return last_cpu_usage;
            
        if (first_run)
        {
            prev_sys_idle = sys_idle;
            prev_sys_kernel = sys_kernel;
            prev_sys_user = sys_user;
            first_run = false;
            return last_cpu_usage;
        }
        
        ULARGE_INTEGER sys_idle_u, sys_kernel_u, sys_user_u;
        sys_idle_u.LowPart = sys_idle.dwLowDateTime; sys_idle_u.HighPart = sys_idle.dwHighDateTime;
        sys_kernel_u.LowPart = sys_kernel.dwLowDateTime; sys_kernel_u.HighPart = sys_kernel.dwHighDateTime;
        sys_user_u.LowPart = sys_user.dwLowDateTime; sys_user_u.HighPart = sys_user.dwHighDateTime;
        
        ULARGE_INTEGER prev_sys_idle_u, prev_sys_kernel_u, prev_sys_user_u;
        prev_sys_idle_u.LowPart = prev_sys_idle.dwLowDateTime; prev_sys_idle_u.HighPart = prev_sys_idle.dwHighDateTime;
        prev_sys_kernel_u.LowPart = prev_sys_kernel.dwLowDateTime; prev_sys_kernel_u.HighPart = prev_sys_kernel.dwHighDateTime;
        prev_sys_user_u.LowPart = prev_sys_user.dwLowDateTime; prev_sys_user_u.HighPart = prev_sys_user.dwHighDateTime;
        
        uint64_t sys_idle_diff = sys_idle_u.QuadPart - prev_sys_idle_u.QuadPart;
        uint64_t sys_kernel_diff = sys_kernel_u.QuadPart - prev_sys_kernel_u.QuadPart;
        uint64_t sys_user_diff = sys_user_u.QuadPart - prev_sys_user_u.QuadPart;
        
        uint64_t total_sys = sys_kernel_diff + sys_user_diff;
        if (total_sys > 0)
            last_cpu_usage = static_cast<float>((total_sys - sys_idle_diff) * 100.0) / static_cast<float>(total_sys);
            
        prev_sys_idle = sys_idle;
        prev_sys_kernel = sys_kernel;
        prev_sys_user = sys_user;
        #elif defined(ADAM_PLATFORM_LINUX)
        static uint64_t prev_idle = 0, prev_total = 0;
        std::ifstream file("/proc/stat");
        if (!file.is_open()) return last_cpu_usage;
        
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        std::string cpu;
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        
        uint64_t idle_time = idle + iowait;
        uint64_t total_time = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
        
        if (prev_total > 0 && total_time > prev_total)
        {
            uint64_t total_diff = total_time - prev_total;
            uint64_t idle_diff = idle_time - prev_idle;
            last_cpu_usage = static_cast<float>(total_diff - idle_diff) * 100.0f / static_cast<float>(total_diff);
        }
        
        prev_idle = idle_time;
        prev_total = total_time;
        #endif

        last_time = now;
        return last_cpu_usage;
    }

    void get_ram_usage_mb(float& used_mb, float& available_mb)
    {
        static float last_used = 0.0f;
        static float last_avail = 0.0f;
        static auto last_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        static bool first_run = true;
        
        if (!first_run && std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() < 500)
        {
            used_mb = last_used;
            available_mb = last_avail;
            return;
        }
        
        first_run = false;
            
        #ifdef ADAM_PLATFORM_WINDOWS
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo))
        {
            last_used = static_cast<float>(static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / 1048576.0);
            last_avail = static_cast<float>(static_cast<double>(memInfo.ullAvailPhys) / 1048576.0);
        }
        #elif defined(ADAM_PLATFORM_LINUX)
        std::ifstream file("/proc/meminfo");
        std::string line;
        uint64_t mem_total = 0, mem_avail = 0;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string key;
            uint64_t kb = 0;
            iss >> key >> kb;
            if (key == "MemTotal:") mem_total = kb;
            else if (key == "MemAvailable:") mem_avail = kb;
        }
        
        if (mem_total > 0 && mem_avail > 0)
        {
            last_used = static_cast<float>(mem_total - mem_avail) / 1000.0f;
            last_avail = static_cast<float>(mem_avail) / 1000.0f;
        }
        #endif

        last_time = now;
        used_mb = last_used;
        available_mb = last_avail;
    }

}
