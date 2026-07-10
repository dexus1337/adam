#include "data/port-types/port-input-replay.hpp"

#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "data/formats/pcap.hpp"
#include "data/formats/rff.hpp"
#include "data/port.hpp"
#include "resources/language-strings.hpp"
#include "module/module-recrep.hpp"

#include <chrono>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <array>
#include <cstring>
#include <algorithm>


namespace adam::modules::recrep
{
    const configuration_parameter_list& port_input_replay::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;

            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container data_format_presets = {};
            
            data_format_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("any"_ct, "any"_ct));
            data_format_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("pcap"_ct, "pcap"_ct));
            data_format_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("rff"_ct, "rff"_ct));

            auto data_format_param = std::make_unique<adam::configuration_parameter_string>("data_format"_ct, "any"_ct, std::move(data_format_presets));
            data_format_param->set_description(language_english, "The recording data format. 'any' will try all known formats (pcap, rff)."_ct);
            data_format_param->set_description(language_german, "Das Aufnahmeformat. 'any' versucht alle bekannten Formate (pcap, rff)."_ct);
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
            speed_param->set_range(0.0, 100.0);
            speed_param->set_description(language_english, "The replay speed relative to the original recording. 0.0 will send the packets instantly."_ct);
            speed_param->set_description(language_german, "Die Wiedergabegeschwindigkeit relativ zur ursprünglichen Aufnahme. 0.0 sendet die Pakete sofort."_ct);
            up->add(std::move(speed_param));
            
            configuration_parameter_string::presets_container timestamp_presets = {};
            
            timestamp_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("current"_ct, "current"_ct));
            timestamp_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("original"_ct, "original"_ct));

            auto timestamp_param = std::make_unique<adam::configuration_parameter_string>("timestamps"_ct, "current"_ct, std::move(timestamp_presets));
            timestamp_param->set_description(language_english, "Use the current system time for the packets or the original timestamps from the recording."_ct);
            timestamp_param->set_description(language_german, "Verwenden Sie die aktuelle Systemzeit für die Pakete oder die ursprünglichen Zeitstempel aus der Aufnahme."_ct);
            up->add(std::move(timestamp_param));

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
     :  port_input(item_name, sizeof(replay_state_buffer_data))
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_input_replay::get_user_parameters());
    }

    port_input_replay::~port_input_replay() {}

    const std::unordered_map<string_hash, string_hashed_ct>& port_input_replay::get_format_map()
    {
        // Format name hash -> file extension (as string_hashed for O(1) comparison in loops).
        // Constructed from string_hashed_ct literals so no string processing occurs at runtime.
        static const std::unordered_map<string_hash, string_hashed_ct> formats =
        {
            { ".pcap"_ct.get_hash(), "pcap"_ct },
            { ".rff"_ct.get_hash(),  "rff"_ct  }
        };
        return formats;
    }

    bool port_input_replay::start() 
    {
        set_state(state_starting);

        m_files.clear();
        m_current_file_index = 0;
        m_is_first_packet = true;

        m_file_format = 0;

        auto user_params = get_parameter<adam::configuration_parameter_list>("user_parameters"_ct);

        m_speed_param       = user_params->get<adam::configuration_parameter_double>("speed"_ct);
        m_mode_param        = user_params->get<adam::configuration_parameter_string>("mode"_ct);
        m_data_format_param = user_params->get<adam::configuration_parameter_string>("data_format"_ct);
        m_timestamps_param  = user_params->get<adam::configuration_parameter_string>("timestamps"_ct);

        auto file_mode = user_params->get<adam::configuration_parameter_string>("file_mode"_ct)->get_value();
        auto path      = user_params->get<adam::configuration_parameter_string>("path"_ct)->get_value();

        // For "any", accept any extension registered in the format map; otherwise match exactly.
        // get_file_format returns the resolved format name hash (e.g. "pcap"_ct) for a given path,
        // or 0 if the file should be excluded. Computing it once here avoids repeated lookups in
        // open_next_file() and try_open_as().
        const bool is_any          = (m_data_format_param->get_value() == "any"_ct);
        const auto& fmap           = get_format_map();
        const string_hash param_fmt = is_any ? 0 : m_data_format_param->get_value().get_hash();
        const string_hashed expected_ext_hashed = is_any
            ? string_hashed{}
            : string_hashed("." + std::string(m_data_format_param->get_value()));

        // Returns the format name hash for the file, or 0 if the extension is not accepted.
        auto get_file_format = [&](const std::string& p) -> string_hash
        {
            const string_hashed ext(std::filesystem::path(p).extension().string());
            if (is_any)
            {
                const auto it = fmap.find(ext.get_hash());
                return (it != fmap.end()) ? it->second.get_hash() : 0;
            }
            return (ext == expected_ext_hashed) ? param_fmt : 0;
        };

        if (file_mode == "single_file"_ct)
        {
            const std::string file = std::string(path);
            const string_hash fh   = get_file_format(file);
            if (fh) m_files.push_back({file, fh});
        }
        else if (file_mode == "multiple_files"_ct)
        {
            size_t start = 0;
            size_t end = path.find(';');
            while (end != std::string::npos)
            {
                std::string file = std::string(path.substr(start, end - start));
                const string_hash fh = get_file_format(file);
                if (!file.empty() && fh) m_files.push_back({std::move(file), fh});
                start = end + 1;
                end = path.find(';', start);
            }
            std::string file = std::string(path.substr(start));
            const string_hash fh = get_file_format(file);
            if (!file.empty() && fh) m_files.push_back({std::move(file), fh});
        }
        else if (file_mode == "directory"_ct)
        {
            try
            {
                for (const auto& entry : std::filesystem::directory_iterator(path.empty() ? "." : path.c_str()))
                {
                    if (entry.is_regular_file())
                    {
                        const string_hash fh = get_file_format(entry.path().string());
                        if (fh) m_files.push_back({entry.path().string(), fh});
                    }
                }
                std::sort(m_files.begin(), m_files.end(),
                    [](const auto& a, const auto& b) { return a.first < b.first; });
            }
            catch (const std::exception&)
            {
                set_state(state_stopped);
                return false;
            }
        }

        if (!open_next_file())
        {
            set_state(state_stopped);
            return false;
        }

        return port_input::start();
    }

    bool port_input_replay::stop() 
    {
        set_state(state_stopping);

        bool result = port_input::stop();

        if (m_file_stream.is_open())
            m_file_stream.close();

        m_files.clear();

        return result;
    }

    bool port_input_replay::try_open_as(string_hash fmt_hash, bool log_errors)
    {
        // Assumes m_file_stream is already open and seeked to position 0.
        // On success: advances m_current_file_index, fills state buffer, sets m_is_first_packet, returns true.
        // On failure: does NOT advance index, returns false (caller is responsible for closing the stream).

        auto lang = get_controller()->get_language();

        switch (fmt_hash)
        {
            case "pcap"_ct:
            {
                pcap::file_header fh;
                m_file_stream.read(reinterpret_cast<char*>(&fh), sizeof(fh));

                if (m_file_stream.gcount() != sizeof(fh))
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(file_too_small, lang)));
                    return false;
                }
                if (fh.magic != pcap::file_header::magic_number)
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(invalid_magic_number, lang)));
                    return false;
                }
                if (fh.ver_maj != pcap::version_major || fh.ver_min != pcap::version_minor)
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(unsupported_version, lang)));
                    return false;
                }

                m_current_file_index++;

                // Scan PCAP to retrieve first/last packet timestamps for the state buffer.
                std::streampos start_pos = m_file_stream.tellg();
                pcap::block_header bh;
                m_file_stream.read(reinterpret_cast<char*>(&bh), sizeof(bh));
                if (m_file_stream.gcount() == sizeof(bh))
                {
                    uint64_t start_ts = static_cast<uint64_t>(bh.ts_sec) * 1000000000ull + static_cast<uint64_t>(bh.ts_usec) * 1000ull;
                    uint64_t end_ts   = start_ts;

                    while (get_state() != state_stopped)
                    {
                        m_file_stream.seekg(bh.incl_len, std::ios::cur);
                        m_file_stream.read(reinterpret_cast<char*>(&bh), sizeof(bh));
                        if (m_file_stream.gcount() != sizeof(bh)) break;
                        end_ts = static_cast<uint64_t>(bh.ts_sec) * 1000000000ull + static_cast<uint64_t>(bh.ts_usec) * 1000ull;
                    }

                    auto* state_data              = get_state_buffer()->data_as<replay_state_buffer_data>();
                    state_data->file_time_start   = start_ts;
                    state_data->file_time_end     = end_ts;
                    state_data->replay_start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                    const std::string& filename = m_files[m_current_file_index - 1].first;
                    std::strncpy(state_data->file_name, filename.c_str(), sizeof(state_data->file_name) - 1);
                    state_data->file_name[sizeof(state_data->file_name) - 1] = '\0';
                }

                m_file_stream.clear();
                m_file_stream.seekg(start_pos);
                m_is_first_packet = true;
                return true;
            }
            case "rff"_ct:
            {
                rff::file_header fh;
                m_file_stream.read(reinterpret_cast<char*>(&fh), sizeof(fh));

                if (m_file_stream.gcount() != sizeof(fh))
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(file_too_small, lang)));
                    return false;
                }
                if (fh.magic != rff::file_header::magic_number)
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(invalid_magic_number, lang)));
                    return false;
                }
                if (fh.version.major != '1')
                {
                    if (log_errors) get_controller()->log(adam::log::error, std::format("Port ({}) [replay]: {}", get_name().c_str(), get_log_event_text(unsupported_version, lang)));
                    return false;
                }

                m_current_file_index++;

                auto start_tm = rff::from_time_string(fh.time_start);
                auto end_tm   = rff::from_time_string(fh.time_end);

                uint64_t start_ts = static_cast<uint64_t>(std::mktime(&start_tm)) * 1000000000ull;
                uint64_t end_ts   = static_cast<uint64_t>(std::mktime(&end_tm))   * 1000000000ull;

                auto* state_data              = get_state_buffer()->data_as<replay_state_buffer_data>();
                state_data->file_time_start   = start_ts;
                state_data->file_time_end     = end_ts;
                state_data->replay_start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                const std::string& filename = m_files[m_current_file_index - 1].first;
                std::strncpy(state_data->file_name, filename.c_str(), sizeof(state_data->file_name) - 1);
                state_data->file_name[sizeof(state_data->file_name) - 1] = '\0';

                m_is_first_packet = true;
                return true;
                break;
            }
        }

        return false;
    }

    bool port_input_replay::open_next_file()
    {
        if (m_file_stream.is_open())
            m_file_stream.close();

        while (get_state() != state_stopped)
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

            // Format hash was already resolved during file collection in start() - no ext lookup needed.
            const auto& [file_path, fmt_hash] = m_files[m_current_file_index];
            m_file_stream.open(file_path, std::ios::binary);

            if (!m_file_stream.is_open())
            {
                m_current_file_index++;
                continue;
            }

            if (try_open_as(fmt_hash, /*log_errors=*/true))
            {
                m_file_format = fmt_hash;
                return true;
            }

            m_file_stream.close();
            m_current_file_index++;
        }
        return false;
    }

    bool port_input_replay::read(buffer*& buff) 
    {
        auto* state_data = get_state_buffer()->data_as<replay_state_buffer_data>();

        if (!m_file_stream.is_open())
        {
            if (!open_next_file())
            {
                set_state(state_inactive);
                return false;
            }
        }

        switch (m_file_format)
        {
            case "pcap"_ct:
            {
                pcap::block_header bh;
                m_file_stream.read(reinterpret_cast<char*>(&bh), sizeof(bh));
                if (m_file_stream.gcount() != sizeof(bh))
                {
                    if (!open_next_file())
                    {
                        set_state(state_inactive);
                        return false;
                    }
                    return read(buff);
                }

                std::chrono::nanoseconds packet_ts(static_cast<uint64_t>(bh.ts_sec) * 1000000000ull + bh.ts_usec * 1000ull);

                // Sleep logic for set speed
                if (m_speed_param->get_value() > 0.0)
                {
                    if (m_is_first_packet)
                    {
                        m_is_first_packet           = false;
                        m_first_packet_timestamp_ns = packet_ts;
                        m_replay_start_time         = std::chrono::steady_clock::now();
                    }
                    else
                    {
                        auto elapsed_packet_time    = packet_ts - m_first_packet_timestamp_ns;
                        auto adjusted_elapsed_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
                        (
                            std::chrono::duration<double, std::nano>(static_cast<double>(elapsed_packet_time.count()) / m_speed_param->get_value())
                        );
                        auto expected_time          = m_replay_start_time + adjusted_elapsed_time;
                        
                        while (is_running())
                        {
                            auto now = std::chrono::steady_clock::now();
                            if (now >= expected_time) break;
                            
                            auto sleep_duration = expected_time - now;
                            if (sleep_duration > std::chrono::milliseconds(5))
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                            }
                            else
                            {
                                std::this_thread::sleep_until(expected_time);
                                break;
                            }
                        }
                    }
                }

                buff = buffer_manager::get().request_buffer(bh.incl_len);
                if (!buff)
                {
                    return false;
                }

                m_file_stream.read(reinterpret_cast<char*>(buff->data()), bh.incl_len);
                buff->set_size(bh.incl_len);
                
                if (m_timestamps_param && m_timestamps_param->get_value() == "original"_ct)
                {
                    buff->set_timestamp(static_cast<uint64_t>(bh.ts_sec) * 1000000000ull + static_cast<uint64_t>(bh.ts_usec) * 1000ull);
                }
                else
                {
                    buff->set_timestamp();
                }

                return true;
            }
            case "rff"_ct:
            {
                rff::block_header bh;
                m_file_stream.read(reinterpret_cast<char*>(&bh), sizeof(bh));
                if (m_file_stream.gcount() != sizeof(bh))
                {
                    if (!open_next_file())
                    {
                        set_state(state_inactive);
                        return false;
                    }
                    return read(buff);
                }

                // time_diff_ms is the absolute ms offset from the start of the recording
                // (file_header.time_start). ALL packets, including the first, are scheduled at
                // m_replay_start_time + time_diff_ms / speed. A packet at offset 0 sends immediately;
                // a packet at offset 5000ms delays 5 seconds - which is the intended behaviour.
                std::chrono::nanoseconds packet_ts_ns(static_cast<uint64_t>(bh.time_diff_ms) * 1000000ull);

                // Sleep logic for set speed
                if (m_speed_param->get_value() > 0.0)
                {
                    if (m_is_first_packet)
                    {
                        // Anchor wall-clock baseline at the moment the first read() is entered.
                        // This absorbs any file-open / header-parse latency without skewing the offset.
                        m_is_first_packet   = false;
                        m_replay_start_time = std::chrono::steady_clock::now();
                    }

                    auto adjusted_elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>
                    (
                        std::chrono::duration<double, std::nano>(static_cast<double>(packet_ts_ns.count()) / m_speed_param->get_value())
                    );
                    auto expected_time = m_replay_start_time + adjusted_elapsed_time;

                    while (is_running())
                    {
                        auto now = std::chrono::steady_clock::now();
                        if (now >= expected_time) break;

                        auto sleep_duration = expected_time - now;
                        if (sleep_duration > std::chrono::milliseconds(5))
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        }
                        else
                        {
                            std::this_thread::sleep_until(expected_time);
                            break;
                        }
                    }
                }

                buff = buffer_manager::get().request_buffer(bh.block_size_bytes);
                if (!buff)
                    return false;

                m_file_stream.read(reinterpret_cast<char*>(buff->data()), bh.block_size_bytes);
                buff->set_size(bh.block_size_bytes);
                
                if (m_timestamps_param && m_timestamps_param->get_value() == "original"_ct)
                {
                    buff->set_timestamp(state_data->file_time_start + packet_ts_ns.count());
                }
                else
                {
                    buff->set_timestamp();
                }

                return true;
            };
        }
        return false;
    }

    std::string_view port_input_replay::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<log_event, std::array<std::string_view, languages_count>> translations =
        {
            {
                log_event::file_too_small,
                { "Failed to read file header. File is too small.", "Fehler beim Lesen des Datei-Headers. Datei ist zu klein." }
            },
            {
                log_event::invalid_magic_number,
                { "Invalid magic number in file.", "Ungültige magische Zahl in der Datei." }
            },
            {
                log_event::unsupported_version,
                { "Unsupported version.", "Nicht unterstützte Version." }
            }
        };

        auto it = translations.find(event);

        if (it != translations.end())
            return it->second[lang];
        
        return language_strings::unknown_type_message("port_input_replay::log_event", static_cast<int>(event), lang);
    }
}