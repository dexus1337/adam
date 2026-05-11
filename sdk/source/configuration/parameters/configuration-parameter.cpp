#include "configuration/parameters/configuration-parameter.hpp"

#include <string>
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"

namespace adam 
{
    configuration_parameter::configuration_parameter()
        : m_str_name() {}

    configuration_parameter::configuration_parameter(const string_hashed& name)
        : m_str_name(name) {}

    configuration_parameter::~configuration_parameter() {}

    void configuration_parameter::write_string(std::ostream& os, const std::string& str) 
    {
        uint32_t len = static_cast<uint32_t>(str.length());
        write_binary(os, len);
        os.write(str.c_str(), len);
    }

    string_hashed configuration_parameter::read_string(std::istream& is) 
    {
        uint32_t len = 0;
        read_binary(is, len);
        std::string str(len, '\0');
        is.read(&str[0], len);
        return string_hashed(str);
    }

    void configuration_parameter::serialize(std::ostream& os, const configuration_parameter* param) 
    {
        if (!param) 
            return;

        write_binary(os, param->get_type());
        write_string(os, std::string(param->get_name()));

        switch (param->get_type()) 
        {
            case configuration_parameter::integer:
                write_binary(os, static_cast<const configuration_parameter_integer*>(param)->get_value());
                break;
            case configuration_parameter::double_:
                write_binary(os, static_cast<const configuration_parameter_double*>(param)->get_value());
                break;
            case configuration_parameter::boolean:
                write_binary(os, static_cast<const configuration_parameter_boolean*>(param)->get_value());
                break;
            case configuration_parameter::string:
                write_string(os, static_cast<const configuration_parameter_string*>(param)->get_value());
                break;
            case configuration_parameter::reference:
                write_string(os, std::string(static_cast<const configuration_parameter_reference*>(param)->get_target()));
                break;
            case configuration_parameter::list: 
            {
                const auto& children = static_cast<const configuration_parameter_list*>(param)->get_children();
                uint32_t count = static_cast<uint32_t>(children.size());
                write_binary(os, count);

                for (const auto& [child_name, child] : children)
                    serialize(os, child.get());
                
                break;
            }
            default: break;
        }
    }

    std::unique_ptr<configuration_parameter> configuration_parameter::deserialize(std::istream& is) 
    {
        if (is.peek() == EOF) 
            return nullptr;

        configuration_parameter::type type;
        read_binary(is, type);
        string_hashed name = read_string(is);

        switch (type) 
        {
            case configuration_parameter::integer: 
            {
                int64_t value;
                read_binary(is, value);
                return std::make_unique<configuration_parameter_integer>(name, value);
            }
            case configuration_parameter::double_: 
            {
                double value;
                read_binary(is, value);
                return std::make_unique<configuration_parameter_double>(name, value);
            }
            case configuration_parameter::boolean: 
            {
                bool value;
                read_binary(is, value);
                return std::make_unique<configuration_parameter_boolean>(name, value);
            }
            case configuration_parameter::string: 
                return std::make_unique<configuration_parameter_string>(name, read_string(is));
            case configuration_parameter::reference: 
                return std::make_unique<configuration_parameter_reference>(name, read_string(is));
            case configuration_parameter::list: 
            {
                auto list_param = std::make_unique<configuration_parameter_list>(name);
                uint32_t count;
                read_binary(is, count);

                for (uint32_t i = 0; i < count; ++i)
                    list_param->add(deserialize(is));

                return list_param;
            }
            default:
                return nullptr;
        }
    }
}