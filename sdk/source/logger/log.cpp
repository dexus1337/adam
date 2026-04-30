#include "logger/log.hpp"


#include <chrono>
#include <algorithm>


namespace adam
{
    log::log(level t, std::string_view txt)
     :  m_level(t)
    {
        if (m_level != invalid)
        {
            m_ui64_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            m_tid            = os::get_current_thread_id();

            size_t count = std::min(txt.size(), max_text_length - 1);

            std::copy_n(txt.data(), count, m_text);

            m_text[count] = '\0';
        }
    }

    log::log() : m_level(invalid) { } // explicitly do not zero out text or other members, causes too much overhead. 

    
    static const auto clock_offset = []() 
    {
        auto sys_now = std::chrono::system_clock::now();
        auto steady_now = std::chrono::steady_clock::now();
        return sys_now.time_since_epoch() - steady_now.time_since_epoch();
    }();

    void stream_log(const adam::log& cr_log, std::ostream& stream)
    {
        // 1. Convert steady_clock nanoseconds to system_clock
        auto tp_steady = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(cr_log.get_timestamp()));
        auto tp_sys = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(tp_steady.time_since_epoch() + clock_offset));

        // 2. Extract time for formatting
        std::time_t tt = std::chrono::system_clock::to_time_t(tp_sys);
        std::tm* local_tm = std::localtime(&tt); // Or gmtime for UTC

        // 3. Extract milliseconds for that extra "high-perf" feel
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_sys.time_since_epoch()) % 1000;

        // 4. Define log level labels and colors (ANSI codes)
        const char* level_str   = nullptr;
        const char* color_code  = nullptr;

        switch (cr_log.get_level()) 
        {
            default: return;
            case log::level::trace:     level_str = "trace";    color_code = "\033[36m";   break; // Cyan
            case log::level::info:      level_str = "info";     color_code = "\033[32m";   break; // Green
            case log::level::warning:   level_str = "warn";     color_code = "\033[33m";   break; // Yellow
            case log::level::error:     level_str = "error";    color_code = "\033[31m";   break; // Red
            case log::level::fatal:     level_str = "fatal";    color_code = "\033[1;31m"; break; // Bold Red
        }

        // 5. The "Beautified" Output
        auto str = std::format
        (   "[{:02}:{:02}:{:02}.{:03}] {}[{}]\033[0m {}\n",
            local_tm->tm_hour,
            local_tm->tm_min,
            local_tm->tm_sec,
            static_cast<int>(ms.count()),
            color_code,
            level_str,
            cr_log.get_text()
        );

        stream << str;
    }
}