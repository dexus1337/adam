#include "data/processor.hpp"

#include "data/format.hpp"

namespace adam 
{
    data_processor::data_processor(const string_hashed& name)
     :  configuration_item(name),
        m_format_input(&data_format_transparent),
        m_format_output(&data_format_transparent)
    {
    }

    data_processor::~data_processor()
    {
        // Clean up resources if necessary (currently none)
    };
}