#include "module/module.hpp"

#include "data/format-can.hpp"
#include "data/can-parser.hpp"
#include "data/filters/can-message-filter.hpp"

static adam::modules::can::module_can global_instance = adam::modules::can::module_can();

static adam::default_factory<adam::processor, adam::modules::can::can_message_filter> can_message_filter_factory = adam::default_factory<adam::processor, adam::modules::can::can_message_filter>();
static adam::default_factory<adam::parser, adam::modules::can::can_parser>             can_parser_factory         = adam::default_factory<adam::parser, adam::modules::can::can_parser>();


namespace adam::modules::can
{
    module_can::module_can() : module("can", version)
    {
        m_data_formats.emplace(data_format_can.get_name(), &data_format_can);

        m_parser_factories.emplace
        (
            data_format_can.get_name(), 
            registry::factory_data_parser(&can_parser_factory, &can_parser::get_user_parameters())
        );

        m_descriptions[static_cast<size_t>(adam::language_english)] = std::string("Provides the Controller Area Network (CAN) data format.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = std::string("Bietet das Controller Area Network (CAN) Datenformat an.");

        m_processor_factories.emplace
        (
            can_message_filter::type_name(), 
            registry::factory_data_processor(&can_message_filter_factory, &can_message_filter::get_user_parameters(), data_format_can.get_name(), this->get_name(), data_format_can.get_name(), this->get_name())
        );
    }

    module_can::~module_can() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &global_instance;
}