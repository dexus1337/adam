#include "data/converter/asterix-to-text-converter.hpp"
#include <cstring>


namespace adam::modules::asterix
{
    asterix_to_text_converter::asterix_to_text_converter(const string_hashed& name) : adam::converter(name) {}

    bool asterix_to_text_converter::handle_data(buffer*& buffer) 
    {
        static const char* test_text_str = "we converted to text";
        
        std::strcpy(buffer->data_as<char>(), test_text_str);

        return true;
    }
}