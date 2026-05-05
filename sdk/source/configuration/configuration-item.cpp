#include "configuration/configuration-item.hpp"


namespace adam 
{
    configuration_item::configuration_item(const string_hashed& name)
     :  m_parameters(name)
    {
        // Initialize resources if necessary (currently none)
    }

    configuration_item::~configuration_item()
    {
        // Clean up resources if necessary (currently none)
    };
}