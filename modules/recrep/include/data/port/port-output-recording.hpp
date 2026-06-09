#pragma once

/**
 * @file    port-output-recording.hpp
 * @author  dexus1337
 * @brief   Defines an output port that allows recording data to files, providing functionality for capturing and saving data in the ADAM system.
 * @version 1.0
 * @date    11.05.2026
 */

 
#include "api/api-recrep.hpp"
 
#include <adam-sdk.hpp>

#include <fstream>
#include <string>
#include <chrono>


namespace adam::modules::recrep
{
    /**
     * @class port_output_recording
     * @brief Defines an output port that allows recording data to files, providing functionality for capturing and saving data in the ADAM system.
     */
    class ADAM_RECREP_API port_output_recording : public port_output
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "recording"_ct; }

        enum log_event
        {
            file_open_failed,
            format_not_implemented
        };

        static std::string_view get_log_event_text(log_event event, language lang);

        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new output port object. */
        port_output_recording(const string_hashed& item_name);

        /** @brief Destroys the output port object and cleans up resources. */
        ~port_output_recording();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };

        /** @brief Prepares the port for recording */
        virtual bool start() override;

        /** @brief Releases resources */
        virtual bool stop() override;

        /** @brief Writes received data to a file. */
        virtual bool write(buffer* buff) override;

    private:

        bool open_next_file();
        void close_current_file(bool acquire_spinlock);
        void finalize_current_file();

        adam::configuration_parameter_string*   m_data_format_param     = nullptr;
        adam::configuration_parameter_string*   m_file_mode_param       = nullptr;
        adam::configuration_parameter_string*   m_chunk_mode_param      = nullptr;
        adam::configuration_parameter_integer*  m_chunk_size_param      = nullptr;
        adam::configuration_parameter_integer*  m_chunk_duration_param  = nullptr;
        adam::configuration_parameter_string*   m_path_param            = nullptr;


        std::ofstream   m_file_stream           = {};

        size_t          m_current_chunk_index   = 0;
        size_t          m_current_chunk_bytes   = 0;
        uint64_t        m_first_packet_timestamp_ns = 0;
        uint64_t        m_last_packet_timestamp_ns  = 0;

        std::tm         m_file_start_tm         = {};
        uint64_t        m_file_start_timestamp_ns = 0;
    };
}