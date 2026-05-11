#pragma once

/**
 * @file    log.hpp
 * @author  dexus1337
 * @brief   Defines a log for the controller
 * @version 1.0
 * @date    28.04.2026
 */

 
#include <cstdint>
#include <string>
#include <format>

#include "api/sdk-api.hpp"
#include "os/os.hpp"


namespace adam 
{
    /**
     * @class log
     * @brief Defines a log for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API log 
    {
    public:

        static constexpr size_t log_size_in_bytes = 512;

        enum level : uint32_t
        {
            invalid = 0,
            trace,
            info,
            warning,
            error,
            fatal
        };

        static size_t get_max_text_length() { return log::max_text_length; }

        /** @brief Constructs a new log object.*/
        log(level t, std::string_view txt);

        /** @brief Constructs a new log object using a format string. */
        template<typename... args_type>
        log(level t, std::format_string<args_type...> fmt, args_type&&... args)
            : log(t, std::format(fmt, std::forward<args_type>(args)...))
        { }

        /** @brief Constructs a new log object using a runtime format string (e.g. from a dictionary). */
        template<typename... args_type>
        requires (sizeof...(args_type) > 0)
        log(level t, std::string_view runtime_fmt, args_type&&... args)
            : log(t, std::vformat(runtime_fmt, std::make_format_args(std::forward<args_type>(args)...)))
        { }

        /** @brief Constructs a new log object.*/
        log();

        /** @brief Destroys the log object*/
        ~log() = default;

        level           get_level()     const { return m_level; }
        uint64_t        get_timestamp() const { return m_ui64_timestamp; }
        os::thread_id   get_thread_id() const { return m_tid; }
        const char*     get_text()      const { return m_text; }

    protected:

        uint64_t        m_ui64_timestamp;
        os::thread_id   m_tid;
        level           m_level;

        static constexpr size_t max_text_length = log_size_in_bytes - sizeof(m_ui64_timestamp) - sizeof(m_tid) - sizeof(m_level); /**< Ensure an log is exactly the wanted size in bytes*/

        char            m_text[max_text_length]; 
    };
    #pragma pack(pop)

    static_assert(sizeof(log) == log::log_size_in_bytes, "log size mismatch");

    
    /** @brief Global function for anyone to use to have logs display in adam style. */
    void stream_log(const adam::log& cr_log, std::ostream& stream);

    /** @brief Global function for anyone to use to have logs display in adam style. */
    inline void stream_log(log::level t, std::string_view txt, std::ostream& stream) { return stream_log(adam::log(t, txt), stream); }

    /** @brief Global function for anyone to use to have logs display in adam style, using a format string. */
    template<typename... args_type>
    inline void stream_log(std::ostream& stream, log::level t, std::format_string<args_type...> fmt, args_type&&... args)
    {
        stream_log(adam::log(t, fmt, std::forward<args_type>(args)...), stream);
    }

    /** @brief Global function for anyone to use to have logs display in adam style, using a runtime format string. */
    template<typename... args_type>
    requires (sizeof...(args_type) > 0)
    inline void stream_log(std::ostream& stream, log::level t, std::string_view runtime_fmt, args_type&&... args)
    {
        stream_log(adam::log(t, runtime_fmt, std::forward<args_type>(args)...), stream);
    }
}
