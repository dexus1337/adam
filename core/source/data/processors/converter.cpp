#include "data/processors/converter.hpp"


namespace adam 
{
    converter::converter(const string_hashed& name)
     :  processor(name)
    {
        get_parameter<configuration_parameter_boolean>("is_filter"_ct)->set_value(false);
    }

    converter::~converter()
    {
        // Clean up resources if necessary (currently none)
    };
}