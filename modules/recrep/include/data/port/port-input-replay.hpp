#pragma once

/**
 * @file    port-input-replay.hpp
 * @author  dexus1337
 * @brief   Defines an input port that allows reading data from replay files, providing functionality for replaying recorded data in the ADAM system.
 * @version 1.0
 * @date    11.05.2026
 */


#include "api/api-recrep.hpp"
#include "commander/messages/message-structs.hpp"
 
#include <adam-sdk.hpp>

#include <cstdint>
#include <fstream>
#include <vector>
#include <chrono>


namespace adam::modules::recrep
{
    /**
     * @class port_input_replay
     * @brief Defines an input port that allows reading data from replay files, providing functionality for replaying recorded data in the ADAM system.
     */
    class ADAM_RECREP_API port_input_replay : public port_input
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "replay"_ct; }

        enum log_event
        {
            file_too_small,
            invalid_magic_number,
            unsupported_version,
            format_not_implemented
        };

        struct replay_state_buffer_data : public adam::port::state_buffer_data
        {
            char file_name[max_name_length];
            uint64_t file_time_start;
            uint64_t file_time_end;
            uint64_t replay_start_time;
        };

        static std::string_view get_log_event_text(log_event event, language lang);

        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new input port object. */
        port_input_replay(const string_hashed& item_name);

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input_replay();
        

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }; /**< THANK YOU MSCV for your really good constexp eval. Thats why we have to do such beautiful things */

        /** @brief Checks for available file(s) */
        virtual bool start() override;

        /** @brief Releases resources */
        virtual bool stop() override;

        /** @brief Protoype function for data input */
        virtual bool read(buffer*& buff) override;

    private:

        adam::configuration_parameter_double* m_speed_param = nullptr;
        adam::configuration_parameter_string* m_mode_param = nullptr;
        adam::configuration_parameter_string* m_data_format_param = nullptr;
        adam::configuration_parameter_string* m_timestamps_param = nullptr;

        bool open_next_file();

        std::vector<std::string> m_files;
        size_t m_current_file_index = 0;
        std::ifstream m_file_stream;
        
        bool m_is_first_packet = true;
        std::chrono::steady_clock::time_point m_replay_start_time;
        std::chrono::microseconds m_first_packet_timestamp_us;
    };
}