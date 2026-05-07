#include "data/processors/filter.hpp"


namespace adam 
{
    filter::filter(const string_hashed& name, const configuration_parameter_list& default_params)
     :  data_processor(name, default_params)
    {
    }

    filter::~filter()
    {
        // Clean up resources if necessary (currently none)
    };
}