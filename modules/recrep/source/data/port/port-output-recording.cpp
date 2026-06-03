#include "data/port/port-output-recording.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"

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

            auto chunk_mode_param = std::make_unique<adam::configuration_parameter_string>("chunk_mode"_ct, "time"_ct, std::move(chunk_mode_param_presets));
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

    port_output_recording::~port_output_recording() {}

    bool port_output_recording::write(buffer* buff) 
    {
        (void)buff;
        return false;
    }
}