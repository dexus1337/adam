#include "data/converter/asterix-to-text-converter.hpp"


#include "data/format-asterix.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module-asterix.hpp"

#include <cstring>


namespace adam::modules::asterix
{
    to_text_converter::to_text_converter(const string_hashed& name) : converter(name) 
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        m_format_input  = &data_format_asterix;
        m_format_output = &data_format_transparent;
    }

    bool to_text_converter::handle_data(buffer*& buf) 
    {
        static const char* test_text_str = "we converted to text";
        
        std::strcpy(buf->data_as<char>(), test_text_str);
        buf->set_size(uint32_t(std::strlen(test_text_str)));

        return true;
    }
}