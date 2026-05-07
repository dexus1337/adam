#include "data/processors/converter.hpp"


namespace adam 
{
    converter::converter(const string_hashed& name, const configuration_parameter_list& default_params)
     :  data_processor(name, default_params)
    {
    }


    converter::~converter()
    {
        // Clean up resources if necessary (currently none)
    };
}