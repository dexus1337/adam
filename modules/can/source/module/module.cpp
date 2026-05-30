#include "module/module.hpp"

#include "data/data-format-can.hpp"


static adam::modules::can::module_can global_instance = adam::modules::can::module_can();


namespace adam::modules::can
{
    module_can::module_can() : module("can", version)
    {
        m_data_formats.emplace(data_format_can.get_name(), &data_format_can);

        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("");

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