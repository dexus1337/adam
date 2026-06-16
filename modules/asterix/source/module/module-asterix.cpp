#include "module/module.hpp"


#include "data/format-asterix.hpp"
#include "data/converter/asterix-to-text-converter.hpp"


namespace adam::modules::asterix
{
    static module_asterix global_instance = adam::modules::asterix::module_asterix();

    static default_factory<processor, to_text_converter> global_processor_factory = default_factory<processor, to_text_converter>();

    module_asterix::module_asterix() : module("asterix", version)
    {
        m_data_formats.emplace(data_format_asterix.get_name(), &data_format_asterix);

        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides the ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) data format.\n"
                        "This is a binary data format for the exchange of air traffic control data.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet das Datenformat ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) an.\n"
                        "Dies ist ein binäres Datenformat für den Austausch von Flugsicherungsdaten.");

        // Export the factory for the controller to dynamically create this port type!
        m_processor_factories.emplace
        (
            to_text_converter::type_name(), 
            registry::factory_data_processor(&global_processor_factory, nullptr /*&to_text_converter::get_user_parameters()*/, data_format_asterix.get_name(), this->get_name(), 0, 0)
        );

    }

    module_asterix::~module_asterix() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &adam::modules::asterix::global_instance;
}
