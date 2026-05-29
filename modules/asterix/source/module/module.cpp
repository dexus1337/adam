#include "module/module.hpp"


#include "data/data-format-asterix.hpp"


static adam::modules::asterix::module_asterix global_instance = adam::modules::asterix::module_asterix();


namespace adam::modules::asterix
{
    module_asterix::module_asterix() : module("asterix", version)
    {
        m_data_formats.emplace(data_format_asterix.get_name(), &data_format_asterix);

        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides the ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) data format.\n"
                        "This is a binary data format for the exchange of air traffic control data.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet das Datenformat ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) an.\n"
                        "Dies ist ein binäres Datenformat für den Austausch von Flugsicherungsdaten.");

    }

    module_asterix::~module_asterix() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}
