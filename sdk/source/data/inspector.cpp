#include "data/inspector.hpp"

namespace adam 
{
    inspector::inspector(const string_hashed& name, const configuration_parameter_list& default_params)
     :  configuration_item(name, default_params)
    {
    }

    inspector::~inspector()
    {
        // Clean up resources if necessary (currently none)
    };
}