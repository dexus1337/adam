#include "data/processors/filter.hpp"


namespace adam 
{
    filter::filter(const string_hashed& name)
     :  data_processor(name)
    {
    }

    filter::~filter()
    {
        // Clean up resources if necessary (currently none)
    };
}