#include "data/processor.hpp"

#include "data/format.hpp"

namespace adam 
{
    data_processor::data_processor(const string_hashed& name, const configuration_parameter_list& default_params)
     :  configuration_item(name, default_params),
        m_format_input(&data_format_transparent),
        m_format_output(&data_format_transparent)
    {
    }

    data_processor::~data_processor()
    {
        // Clean up resources if necessary (currently none)
    };
}