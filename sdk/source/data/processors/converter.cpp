#include "data/processors/converter.hpp"


namespace adam 
{
    converter::converter(const string_hashed& name)
     :  data_processor(name)
    {
    }


    converter::~converter()
    {
        // Clean up resources if necessary (currently none)
    };
}