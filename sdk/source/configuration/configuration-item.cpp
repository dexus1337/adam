#include "configuration/configuration-item.hpp"


namespace adam 
{
    configuration_item::configuration_item(const string_hashed& name, const configuration_parameter_list& default_params)
     :  m_parameters(default_params) // Utilizes the deep-copy constructor!
    {
        m_parameters.m_str_name = name;
    }

    configuration_item::~configuration_item()
    {
        // Clean up resources if necessary (currently none)
    };
}