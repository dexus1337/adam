#include "module/module-recrep.hpp"


#include "data/port-types/port-input-replay.hpp"
#include "data/port-types/port-output-recording.hpp"

namespace adam::modules::recrep
{
    static module_recrep global_instance = module_recrep();

    static default_factory<port, port_input_replay>     global_port_input_replay_factory        = default_factory<port, port_input_replay>();
    static default_factory<port, port_output_recording> global_port_output_recording_factory    = default_factory<port, port_output_recording>();

    module_recrep::module_recrep() : module("recrep", version)
    {
        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides capabilities for recording and replaying data streams.\n"
            "Both recordings and replays are format-independent." );
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet Funktionen zum Aufzeichnen und Abspielen von Datenströmen.\n"
            "Sowohl Aufzeichnungen als auch Wiedergaben sind formatunabhängig." );

        // Export the factory for the controller to dynamically create this port type!
        m_port_factories.emplace
        (
            port_input_replay::type_name(), 
            registry::factory_data_port(&global_port_input_replay_factory, &port_input_replay::get_default_parameters(), port_input_replay::direction)
        );

        m_port_factories.emplace
        (
            port_output_recording::type_name(), 
            registry::factory_data_port(&global_port_output_recording_factory, &port_output_recording::get_default_parameters(), port_output_recording::direction)
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