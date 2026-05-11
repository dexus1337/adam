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

    std::string get_log_time_string(uint64_t timestamp)
    {
        auto tp_steady = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(timestamp));
        auto tp_sys = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(tp_steady.time_since_epoch() + clock_offset));

        std::time_t tt = std::chrono::system_clock::to_time_t(tp_sys);
        std::tm* local_tm = std::localtime(&tt); // Or gmtime for UTC

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_sys.time_since_epoch()) % 1000;

        return std::format("[{:02}:{:02}:{:02}.{:03}]", local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec, static_cast<int>(ms.count()));
    }

    void get_log_appearance(log::level level, const char*& level_str, float& r, float& g, float& b)
    {
        switch (level) 
        {
            default:                  level_str = "unkwn"; r = 1.0f; g = 1.0f; b = 1.0f; break;
            case log::level::trace:   level_str = "trace"; r = 0.0f; g = 1.0f; b = 1.0f; break; // Cyan
            case log::level::info:    level_str = "info";  r = 0.0f; g = 1.0f; b = 0.0f; break; // Green
            case log::level::warning: level_str = "warn";  r = 1.0f; g = 1.0f; b = 0.0f; break; // Yellow
            case log::level::error:   level_str = "error"; r = 1.0f; g = 0.0f; b = 0.0f; break; // Red
            case log::level::fatal:   level_str = "fatal"; r = 1.0f; g = 0.0f; b = 0.0f; break; // Bold Red
        }
    }

    void stream_log(const adam::log& cr_log, std::ostream& stream)
    {
        const char* level_str   = nullptr;
        float r, g, b;
        get_log_appearance(cr_log.get_level(), level_str, r, g, b);

        const char* color_code  = nullptr;

        switch (cr_log.get_level()) 
        {
            default: return;
            case log::level::trace:     color_code = "\033[36m";   break; // Cyan
            case log::level::info:      color_code = "\033[32m";   break; // Green
            case log::level::warning:   color_code = "\033[33m";   break; // Yellow
            case log::level::error:     color_code = "\033[31m";   break; // Red
            case log::level::fatal:     color_code = "\033[1;31m"; break; // Bold Red
        }

        // 5. The "Beautified" Output
        auto str = std::format
        (   "{} {}[{}]\033[0m {}\n",
            get_log_time_string(cr_log.get_timestamp()),
            color_code,
            level_str,
            cr_log.get_text()
        );

        stream << str;
    }
}