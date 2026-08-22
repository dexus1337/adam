#include "data/processors/filter.hpp"


namespace adam 
{
    filter::filter(const string_hashed& name)
     :  processor(name)
    {
        get_parameter<configuration_parameter_boolean>("is_filter"_ct)->set_value(true);
    }

    filter::~filter()
    {
        // Clean up resources if necessary (currently none)
    };
}