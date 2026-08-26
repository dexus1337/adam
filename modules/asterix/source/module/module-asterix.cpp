#include "module/module-asterix.hpp"


#include "data/format-asterix.hpp"
#include "data/converters/asterix-to-text-converter.hpp"
#include "data/filters/asterix-sac-sic-replacer.hpp"
#include "data/filters/asterix-category-filter.hpp"
#include "data/asterix-parser.hpp"
#include "data/asterix-encoder.hpp"

namespace adam::modules::asterix
{
    static module_asterix global_instance = adam::modules::asterix::module_asterix();

    static default_factory<processor,   category_filter>    category_filter_factory     = default_factory<processor,  category_filter>();
    static default_factory<processor,   to_text_converter>  to_text_converter_factory   = default_factory<processor,  to_text_converter>();
    static default_factory<processor,   sac_sic_replacer>   sac_sic_filter_factory      = default_factory<processor,  sac_sic_replacer>();
    static default_factory<parser,      asterix_parser>     asterix_parser_factory      = default_factory<parser,     asterix_parser>();
    static default_factory<encoder,     asterix_encoder>    asterix_encoder_factory     = default_factory<encoder,    asterix_encoder>();

    module_asterix::module_asterix() : module("asterix", version)
    {
        m_data_formats.emplace(data_format_asterix.get_name(), &data_format_asterix);

        m_parser_factories.emplace
        (
            data_format_asterix.get_name(), 
            registry::factory_data_parser(&asterix_parser_factory, &asterix_parser::get_user_parameters())
        );

        m_encoder_factories.emplace
        (
            data_format_asterix.get_name(), 
            registry::factory_data_encoder(&asterix_encoder_factory, &asterix_encoder::get_user_parameters())
        );

        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides the ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) data format.\n"
                        "This is a binary data format for the exchange of air traffic control data.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet das Datenformat ASTERIX (All Purpose Structured Eurocontrol Surveillance Information Exchange) an.\n"
                        "Dies ist ein binäres Datenformat für den Austausch von Flugsicherungsdaten.");

        // Export the factory for the controller to dynamically create this port type!
        m_processor_factories.emplace
        (
            category_filter::type_name(), 
            registry::factory_data_processor(&category_filter_factory, &category_filter::get_user_parameters(), data_format_asterix.get_name(), this->get_name(), data_format_asterix.get_name(), this->get_name())
        );

        m_processor_factories.emplace
        (
            to_text_converter::type_name(), 
            registry::factory_data_processor(&to_text_converter_factory, nullptr /*&to_text_converter::get_user_parameters()*/, data_format_asterix.get_name(), this->get_name(), 0, 0)
        );

        m_processor_factories.emplace
        (
            sac_sic_replacer::type_name(), 
            registry::factory_data_processor(&sac_sic_filter_factory, &sac_sic_replacer::get_user_parameters(), data_format_asterix.get_name(), this->get_name(), data_format_asterix.get_name(), this->get_name())
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
