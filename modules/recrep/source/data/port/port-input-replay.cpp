#include "data/port/port-input-replay.hpp"

#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

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
        return port_input::start();
    }

    bool port_input_replay::stop() 
    {
        return port_input::stop();
    }

    bool port_input_replay::read(buffer*& buff) 
    {
        (void)buff;
        return true;
    }
}