#include "data/port/port-output-recording.hpp"

#include "configuration/configuration-item.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "controller/controller.hpp"
#include "data/port/port-output.hpp"
#include "resources/language-strings.hpp"
#include "data/formats/pcap.hpp"

#include <filesystem>
#include <format>
#include <unordered_map>
#include <array>
#include <iomanip>
#include <sstream>

namespace adam::modules::recrep
{
    const configuration_parameter_list& port_output_recording::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;

            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container data_format_presets = {};
            
            data_format_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("pcap"_ct, "pcap"_ct));
            data_format_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("rff"_ct, "rff"_ct));

            auto data_format_param = std::make_unique<adam::configuration_parameter_string>("data_format"_ct, "pcap"_ct, std::move(data_format_presets));
            data_format_param->set_description(language_english, "The recording data format."_ct);
            data_format_param->set_description(language_german, "Das Aufnahmeformat."_ct);
            up->add(std::move(data_format_param));

            configuration_parameter_string::presets_container file_mode_param_presets = {};
            
            file_mode_param_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("single_file"_ct, "single_file"_ct));
            file_mode_param_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("chunked_file"_ct, "chunked file"_ct));

            auto file_mode_param = std::make_unique<adam::configuration_parameter_string>("file_mode"_ct, "single_file"_ct, std::move(file_mode_param_presets));
            file_mode_param->set_description(language_english, "The mode of operation for the output port, either writing to a single file or chunked files (by time)."_ct);
            file_mode_param->set_description(language_german, "Der Betriebsmodus für den Ausgangsport, entweder Schreiben in eine einzelne Datei oder mehrere Dateien (nach Zeit)."_ct);
            up->add(std::move(file_mode_param));

            auto path_param = std::make_unique<adam::configuration_parameter_string>("path"_ct);
            path_param->set_description(language_english, "The path where the recording file(s) are stored."_ct);
            path_param->set_description(language_german, "Der Pfad wo die Aufnahmedatei(en) gespeichert werden."_ct);
            up->add(std::move(path_param));

            configuration_parameter_string::presets_container chunk_mode_param_presets = {};
            
            chunk_mode_param_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("size"_ct, "size"_ct));
            chunk_mode_param_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("time"_ct, "time"_ct));

            auto chunk_mode_param = std::make_unique<adam::configuration_parameter_string>("chunk_mode"_ct, "size"_ct, std::move(chunk_mode_param_presets));
            chunk_mode_param->set_description(language_english, "The mode of operation for chunked files, either by size or by time."_ct);
            chunk_mode_param->set_description(language_german, "Der Betriebsmodus für geteilte Dateien, entweder nach Größe (size) oder nach Zeit (time)."_ct);
            up->add(std::move(chunk_mode_param));

            auto chunk_size_param = std::make_unique<adam::configuration_parameter_integer>("chunk_size"_ct, 5);
            chunk_size_param->set_mode(configuration_parameter_integer::value_mode_range);
            chunk_size_param->set_range(1, 100000);
            chunk_size_param->set_description(language_english, "The size of each chunk in megabytes. Only used when chunk mode is size"_ct);
            chunk_size_param->set_description(language_german, "Die Größe jeder Datei in Megabyte. Nur verwendet wenn der Chunk Modus nach Größe (size) ist."_ct);
            up->add(std::move(chunk_size_param));

            auto chunk_duration_param = std::make_unique<adam::configuration_parameter_integer>("chunk_duration"_ct, 300);
            chunk_duration_param->set_mode(configuration_parameter_integer::value_mode_range);
            chunk_duration_param->set_range(1, 3600);
            chunk_duration_param->set_description(language_english, "The duration of each chunk in seconds. Only used when chunk mode is time."_ct);
            chunk_duration_param->set_description(language_german, "Die Dauer jeder Datei in Sekunden. Nur verwendet wenn der Chunk Modus nach Zeit (time) ist."_ct);
            up->add(std::move(chunk_duration_param));

            p.add(std::move(up));
            
            return p;
        }();
        return params;
    }

    port_output_recording::port_output_recording(const string_hashed& item_name) 
     :  port_output(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_output_recording::get_default_parameters());
    }

    port_output_recording::~port_output_recording() 
    {
        if (m_file_stream.is_open())
        {
            m_file_stream.flush();
            m_file_stream.close();
        }
    }

    bool port_output_recording::start()
    {
        auto user_params = get_parameter<adam::configuration_parameter_list>("user_parameters"_ct);

        m_data_format_param = user_params->get<adam::configuration_parameter_string>("data_format"_ct);
        m_file_mode_param = user_params->get<adam::configuration_parameter_string>("file_mode"_ct);
        m_chunk_mode_param = user_params->get<adam::configuration_parameter_string>("chunk_mode"_ct);
        m_chunk_size_param = user_params->get<adam::configuration_parameter_integer>("chunk_size"_ct);
        m_chunk_duration_param = user_params->get<adam::configuration_parameter_integer>("chunk_duration"_ct);
        m_path_param = user_params->get<adam::configuration_parameter_string>("path"_ct);

        m_current_chunk_index = 0;
        
        if (!open_next_file()) 
            return false;

        return port_output::start();
    }

    bool port_output_recording::stop()
    {
        if (m_file_stream.is_open())
        {
            m_file_stream.flush();
            m_file_stream.close();
        }

        return port_output::stop();
    }

    bool port_output_recording::open_next_file()
    {
        if (m_file_stream.is_open())
        {
            m_file_stream.flush();
            m_file_stream.close();
        }

        auto lang = adam::controller::get().get_language();

        if (m_data_format_param->get_value() == "rff"_ct)
        {
            adam::controller::get().log(adam::log::error, get_log_event_text(format_not_implemented, lang));
            return false;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm_info;
        #ifdef _WIN32
        localtime_s(&tm_info, &time);
        #else
        localtime_r(&time, &tm_info);
        #endif
        std::ostringstream oss;
        oss << std::put_time(&tm_info, "%d%m%y_%H%M%S_") << std::setfill('0') << std::setw(3) << ms.count();
        std::string time_str = oss.str();

        std::string extension = "." + m_data_format_param->get_value();

        std::string file_name;
        if (m_file_mode_param->get_value() == "chunked_file"_ct)
        {
            file_name = std::format("{}_{}_{:03}{}", configuration_item::get_name().c_str(), time_str, m_current_chunk_index, extension);
        }
        else
        {
            file_name = std::format("{}_{}{}", configuration_item::get_name().c_str(), time_str, extension);
        }

        std::string final_path;
        if (m_path_param->get_value().empty())
        {
            final_path = file_name;
        }
        else
        {
            final_path = (std::filesystem::path(m_path_param->get_value().c_str()) / file_name).string();
        }

        m_file_stream.open(final_path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!m_file_stream.is_open())
        {
            adam::controller::get().log(adam::log::error, get_log_event_text(file_open_failed, lang));
            return false;
        }

        m_current_chunk_bytes = 0;
        m_first_packet_timestamp_ns = 0;

        if (m_data_format_param->get_value() == "pcap"_ct)
        {
            pcap::file_header fh = {};
            fh.magic = pcap::file_header::magic_number;
            fh.ver_maj = pcap::version_major;
            fh.ver_min = pcap::version_minor;
            fh.thiszone = 0;
            fh.sigfigs = 0;
            fh.snaplen = 65535;
            fh.network = pcap::network_type::raw;
            
            m_file_stream.write(reinterpret_cast<const char*>(&fh), sizeof(fh));
            m_current_chunk_bytes += sizeof(fh);
        }

        m_current_chunk_index++;

        return true;
    }

    bool port_output_recording::write(buffer* buff) 
    {
        if (!buff || !m_file_stream.is_open()) 
            return false;

        bool is_chunked = m_file_mode_param->get_value() == "chunked_file"_ct;
        
        if (is_chunked)
        {
            auto chunk_mode = m_chunk_mode_param->get_value();
            bool should_chunk = false;

            if (chunk_mode == "size"_ct)
            {
                uint64_t max_size_bytes = m_chunk_size_param->get_value() * 1024ull * 1024ull;
                if (m_current_chunk_bytes + sizeof(pcap::packet_header) + buff->get_size() >= max_size_bytes)
                {
                    should_chunk = true;
                }
            }
            else if (chunk_mode == "time"_ct)
            {
                if (m_first_packet_timestamp_ns == 0)
                {
                    m_first_packet_timestamp_ns = buff->get_timestamp();
                }
                else
                {
                    uint64_t max_duration_ns = m_chunk_duration_param->get_value() * 1000000000ull;
                    if (buff->get_timestamp() >= m_first_packet_timestamp_ns + max_duration_ns)
                    {
                        should_chunk = true;
                    }
                }
            }

            if (should_chunk)
            {
                if (!open_next_file())
                {
                    return false;
                }
                if (chunk_mode == "time"_ct)
                {
                    m_first_packet_timestamp_ns = buff->get_timestamp();
                }
            }
        }

        if (m_data_format_param->get_value() == "pcap"_ct)
        {
            pcap::packet_header ph = {};
            uint64_t ts = buff->get_timestamp();
            ph.ts_sec = static_cast<uint32_t>(ts / 1000000000ull);
            ph.ts_usec = static_cast<uint32_t>((ts % 1000000000ull) / 1000ull);
            ph.incl_len = static_cast<uint32_t>(buff->get_size());
            ph.orig_len = static_cast<uint32_t>(buff->get_size());

            m_file_stream.write(reinterpret_cast<const char*>(&ph), sizeof(ph));
            m_file_stream.write(reinterpret_cast<const char*>(buff->data()), buff->get_size());

            m_current_chunk_bytes += sizeof(ph) + buff->get_size();
        }
        else if (m_data_format_param->get_value() == "rff"_ct)
        {
            // TODO: implement rff packet writing logic
            
            // m_file_stream.write(...);
            // m_current_chunk_bytes += buff->get_size() + header_size;
        }

        return true;
    }

    std::string_view port_output_recording::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<log_event, std::array<std::string_view, languages_count>> translations =
        {
            {
                log_event::file_open_failed,
                { "Recording: Failed to open output file.", "Aufnahme: Fehler beim Öffnen der Ausgabedatei." }
            },
            {
                log_event::format_not_implemented,
                { "Recording: Selected data format is not yet implemented.", "Aufnahme: Das ausgewählte Datenformat ist noch nicht implementiert." }
            }
        };

        auto it = translations.find(event);
        if (it != translations.end())
            return it->second[lang];
        
        return language_strings::unknown_type_message("port_output_recording::log_event", static_cast<int>(event), lang);
    }
}