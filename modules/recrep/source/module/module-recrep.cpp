#include "module/module-recrep.hpp"


#include "data/port/port-input-replay.hpp"
#include "data/port/port-output-recording.hpp"

namespace adam::modules::recrep
{
    static module_recrep global_instance = module_recrep();

    static default_factory<port, port_input_replay>     global_port_input_replay_factory        = default_factory<port, port_input_replay>();
    static default_factory<port, port_output_recording> global_port_output_recording_factory    = default_factory<port, port_output_recording>();

    module_recrep::module_recrep() : module("recrep", version)
    {
        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides capabilities for recording and replaying data streams.\n"
            "Supported Input Ports: \"") + port_input_replay::type_name.c_str() + "\"\n"
            "Supported Output Ports: \"" + port_output_recording::type_name.c_str() + "\"\n"
            "Recordings are data-format independent.";
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet Funktionen zum Aufzeichnen und Abspielen von Datenströmen.\n"
            "Unterstützte Eingangsports: \"") + port_input_replay::type_name.c_str() + "\"\n"
            "Unterstützte Ausgangsports: \"" + port_output_recording::type_name.c_str() + "\"\n"
            "Aufzeichnungen sind datenformatunabhängig.";

        // Export the factory for the controller to dynamically create this port type!
        m_port_factories.emplace
        (
            port_input_replay::type_name, 
            &global_port_input_replay_factory
        );

        m_port_factories.emplace
        (
            port_output_recording::type_name, 
            &global_port_output_recording_factory
        );
    }

    module_recrep::~module_recrep() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &adam::modules::recrep::global_instance;
}