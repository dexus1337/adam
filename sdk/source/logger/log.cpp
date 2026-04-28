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
}