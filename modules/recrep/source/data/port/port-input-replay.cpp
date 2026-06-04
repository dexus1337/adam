#include "data/port/port-input-replay.hpp"

#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "data/formats/pcap.hpp"
#include "resources/language-strings.hpp"
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <array>


namespace adam::modules::recrep
{
    const configuration_parameter_list& port_input_replay::get_default_parameters()
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
            file_mode_param_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("multiple_files"_ct, "multiple_files"_ct));
            file_mode_param_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("directory"_ct, "directory"_ct));

            auto file_mode_param = std::make_unique<adam::configuration_parameter_string>("file_mode"_ct, "single_file"_ct, std::move(file_mode_param_presets));
            file_mode_param->set_description(language_english, "The mode of operation for the input port, either reading from a single file, multiple files (separated by semicolons) or from a directory of files."_ct);
            file_mode_param->set_description(language_german, "Der Betriebsmodus für den Eingangsport, entweder Lesen aus einer einzelnen Datei, mehreren Dateien (getrennt durch Semikolon) oder aus einem Verzeichnis von Dateien."_ct);
            up->add(std::move(file_mode_param));

            auto path_param = std::make_unique<adam::configuration_parameter_string>("path"_ct);
            path_param->set_description(language_english, "The path to the recording file(s) or the directory."_ct);
            path_param->set_description(language_german, "Der Pfad zu Aufnahmedatei(en) oder das Verzeichnis."_ct);
            up->add(std::move(path_param));

            auto speed_param = std::make_unique<adam::configuration_parameter_double>("speed"_ct, 1.0);
            speed_param->set_mode(configuration_parameter_double::value_mode_range);
            speed_param->set_range(0.0, 100.0);
            speed_param->set_description(language_english, "The replay speed relative to the original recording. 0.0 will send the packets instantly."_ct);
            speed_param->set_description(language_german, "Die Wiedergabegeschwindigkeit relativ zur ursprünglichen Aufnahme. 0.0 sendet die Pakete sofort."_ct);
            up->add(std::move(speed_param));
            
            configuration_parameter_string::presets_container mode_presets = {};
            
            mode_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("once"_ct, "once"_ct));
            mode_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("loop"_ct, "loop"_ct));

            auto mode_param = std::make_unique<adam::configuration_parameter_string>("mode"_ct, "once"_ct, std::move(mode_presets));
            mode_param->set_description(language_english, "The mode of operation for the input port. 'once' means that the input port will stop after all files have been replayed. 'loop' means that the input port will loop through all files until stopped."_ct);
            mode_param->set_description(language_german, "Der Betriebsmodus für den Eingangsport. 'once' bedeutet, dass der Eingangsport stoppt, nachdem alle Dateien wiedergegeben wurden. 'loop' bedeutet, dass der Eingangsport durch alle Dateien läuft, bis er gestoppt wird."_ct);
            up->add(std::move(mode_param));

            p.add(std::move(up));
            
            return p;
        }();
        return params;
    }

    port_input_replay::port_input_replay(const string_hashed& item_name) 
     :  port_input(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_input_replay::get_default_parameters());
    }

    port_input_replay::~port_input_replay() {}

    bool port_input_replay::start() 
    {
        m_files.clear();
        m_current_file_index = 0;
        m_is_first_packet = true;

        auto user_params = get_parameter<adam::configuration_parameter_list>("user_parameters"_ct);

        m_speed_param = user_params->get<adam::configuration_parameter_double>("speed"_ct);
        m_mode_param = user_params->get<adam::configuration_parameter_string>("mode"_ct);
        m_data_format_param = user_params->get<adam::configuration_parameter_string>("data_format"_ct);

        auto file_mode = user_params->get<adam::configuration_parameter_string>("file_mode"_ct)->get_value();
        auto path = user_params->get<adam::configuration_parameter_string>("path"_ct)->get_value();

        std::string expected_ext = "." + m_data_format_param->get_value();

        auto is_valid_ext = [&](const std::string& p) 
        {
            return std::filesystem::path(p).extension().string() == expected_ext;
        };

        if (file_mode == "single_file"_ct)
        {
            std::string file = std::string(path);
            if (is_valid_ext(file))
                m_files.push_back(file);
        }
        else if (file_mode == "multiple_files"_ct)
        {
            size_t start = 0;
            size_t end = path.find(';');
            while (end != std::string::npos)
            {
                std::string file = std::string(path.substr(start, end - start));
                if (!file.empty() && is_valid_ext(file))
                    m_files.push_back(file);
                start = end + 1;
                end = path.find(';', start);
            }
            std::string file = std::string(path.substr(start));
            if (!file.empty() && is_valid_ext(file))
                m_files.push_back(file);
        }
        else if (file_mode == "directory"_ct)
        {
            try
            {
                for (const auto& entry : std::filesystem::directory_iterator(path.empty() ? "." : path.c_str()))
                {
                    if (entry.is_regular_file())
                    {
                        if (entry.path().extension().string() == expected_ext)
                        {
                            m_files.push_back(entry.path().string());
                        }
                    }
                }
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        if (!open_next_file())
            return false;

        return port_input::start();
    }

    bool port_input_replay::stop() 
    {
        if (m_file_stream.is_open())
        {
            m_file_stream.close();
        }
        m_files.clear();
        return port_input::stop();
    }

    bool port_input_replay::open_next_file()
    {
        if (m_file_stream.is_open())
        {
            m_file_stream.close();
        }

        while (true)
        {
            if (m_current_file_index >= m_files.size())
            {
                if (m_mode_param && m_mode_param->get_value() == "loop"_ct)
                {
                    m_current_file_index = 0;
                    if (m_files.empty()) return false;
                    m_is_first_packet = true;
                }
                else
                {
                    return false;
                }
            }

            m_file_stream.open(m_files[m_current_file_index], std::ios::binary);
            if (m_file_stream.is_open())
            {
                auto lang = adam::controller::get().get_language();

                if (m_data_format_param->get_value() == "pcap"_ct)
                {
                    pcap::file_header fh;
                    m_file_stream.read(reinterpret_cast<char*>(&fh), sizeof(fh));
                    
                    if (m_file_stream.gcount() != sizeof(fh))
                    {
                        adam::controller::get().log(adam::log::error, get_log_event_text(file_too_small, lang));
                    }
                    else if (fh.magic != pcap::file_header::magic_number)
                    {
                        adam::controller::get().log(adam::log::error, get_log_event_text(invalid_magic_number, lang));
                    }
                    else if (fh.ver_maj != pcap::version_major || fh.ver_min != pcap::version_minor)
                    {
                        adam::controller::get().log(adam::log::error, get_log_event_text(unsupported_version, lang));
                    }
                    else
                    {
                        m_current_file_index++;
                        return true;
                    }
                }
                else if (m_data_format_param->get_value() == "rff"_ct)
                {
                    adam::controller::get().log(adam::log::error, get_log_event_text(format_not_implemented, lang));
                }

                m_file_stream.close();
            }
            m_current_file_index++;
        }
        return false;
    }

    bool port_input_replay::read(buffer*& buff) 
    {
        if (!m_file_stream.is_open())
        {
            if (!open_next_file())
            {
                m_is_active->set_value(false);
                return false;
            }
        }

        if (m_data_format_param->get_value() == "pcap"_ct)
        {
            pcap::packet_header ph;
            m_file_stream.read(reinterpret_cast<char*>(&ph), sizeof(ph));
            if (m_file_stream.gcount() != sizeof(ph))
            {
                if (!open_next_file())
                {
                    m_is_active->set_value(false);
                    return false;
                }
                return read(buff);
            }

            double speed = m_speed_param ? m_speed_param->get_value() : 1.0;
            
            std::chrono::microseconds packet_ts(static_cast<uint64_t>(ph.ts_sec) * 1000000ull + ph.ts_usec);

            if (speed > 0.0)
            {
                if (m_is_first_packet)
                {
                    m_is_first_packet = false;
                    m_first_packet_timestamp_us = packet_ts;
                    m_replay_start_time = std::chrono::steady_clock::now();
                }
                else
                {
                    auto elapsed_packet_time = packet_ts - m_first_packet_timestamp_us;
                    auto adjusted_elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::micro>(static_cast<double>(elapsed_packet_time.count()) / speed));
                    auto expected_time = m_replay_start_time + adjusted_elapsed_time;
                    
                    std::this_thread::sleep_until(expected_time);
                }
            }

            buff = buffer_manager::get().request_buffer(ph.incl_len);
            if (!buff)
            {
                return false;
            }

            m_file_stream.read(reinterpret_cast<char*>(buff->data()), ph.incl_len);
            buff->set_size(ph.incl_len);
            buff->set_timestamp(static_cast<uint64_t>(ph.ts_sec) * 1000000000ull + static_cast<uint64_t>(ph.ts_usec) * 1000ull);

            return true;
        }
        else if (m_data_format_param->get_value() == "rff"_ct)
        {
            m_is_active->set_value(false);
            return false;
        }

        return false;
    }

    std::string_view port_input_replay::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<log_event, std::array<std::string_view, languages_count>> translations =
        {
            {
                log_event::file_too_small,
                { "PCAP Replay: Failed to read file header. File is too small.", "PCAP Replay: Fehler beim Lesen des Datei-Headers. Datei ist zu klein." }
            },
            {
                log_event::invalid_magic_number,
                { "PCAP Replay: Invalid magic number in PCAP file.", "PCAP Replay: Ungültige magische Zahl in der PCAP-Datei." }
            },
            {
                log_event::unsupported_version,
                { "PCAP Replay: Unsupported PCAP version.", "PCAP Replay: Nicht unterstützte PCAP-Version." }
            },
            {
                log_event::format_not_implemented,
                { "Replay: Selected data format is not yet implemented.", "Replay: Das ausgewählte Datenformat ist noch nicht implementiert." }
            }
        };

        auto it = translations.find(event);

        if (it != translations.end())
            return it->second[lang];
        
        return language_strings::unknown_type_message("port_input_replay::log_event", static_cast<int>(event), lang);
    }
}