#include "configuration/parameters/configuration-parameter.hpp"

#include <string>
#include <algorithm>
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam 
{
    configuration_parameter::configuration_parameter()
        : m_str_name() {}

    configuration_parameter::configuration_parameter(const string_hashed& name)
        : m_str_name(name) {}

    configuration_parameter::~configuration_parameter() {}

    const string_hashed& configuration_parameter::get_description(language lang) const 
    {
        auto it = m_descriptions.find(lang);
        if (it != m_descriptions.end())
            return it->second;

        static const string_hashed empty_str("");
        return empty_str;
    }

    void configuration_parameter::set_description(language lang, const string_hashed& description) 
    {
        m_descriptions[lang] = description;
    }

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
            case configuration_parameter::type_integer:
                write_binary(os, static_cast<const configuration_parameter_integer*>(param)->get_value());
                break;
            case configuration_parameter::type_double:
                write_binary(os, static_cast<const configuration_parameter_double*>(param)->get_value());
                break;
            case configuration_parameter::type_boolean:
                write_binary(os, static_cast<const configuration_parameter_boolean*>(param)->get_value());
                break;
            case configuration_parameter::type_string:
                write_string(os, static_cast<const configuration_parameter_string*>(param)->get_value());
                break;
            case configuration_parameter::type_reference:
                write_string(os, std::string(static_cast<const configuration_parameter_reference*>(param)->get_target()));
                break;
            case configuration_parameter::type_list: 
            {
                if (auto* sorted_list = dynamic_cast<const configuration_parameter_list_sorted*>(param))
                {
                    const auto& children = sorted_list->get_children();
                    const auto& order = sorted_list->get_order();
                    uint32_t count = static_cast<uint32_t>(children.size());
                    write_binary(os, count);

                    for (string_hash child_hash : order)
                    {
                        auto it = std::find_if(children.begin(), children.end(), [child_hash](const auto& pair) {
                            return pair.first.get_hash() == child_hash;
                        });
                        if (it != children.end())
                        {
                            serialize(os, it->second.get());
                        }
                    }
                }
                else
                {
                    const auto& children = static_cast<const configuration_parameter_list*>(param)->get_children();
                    uint32_t count = static_cast<uint32_t>(children.size());
                    write_binary(os, count);

                    for (const auto& [child_name, child] : children)
                        serialize(os, child.get());
                }
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

        std::unique_ptr<configuration_parameter> result = nullptr;

        switch (type) 
        {
            case configuration_parameter::type_integer: 
            {
                int64_t value;
                read_binary(is, value);
                result = std::make_unique<configuration_parameter_integer>(name, value);
                break;
            }
            case configuration_parameter::type_double: 
            {
                double value;
                read_binary(is, value);
                result = std::make_unique<configuration_parameter_double>(name, value);
                break;
            }
            case configuration_parameter::type_boolean: 
            {
                bool value;
                read_binary(is, value);
                result = std::make_unique<configuration_parameter_boolean>(name, value);
                break;
            }
            case configuration_parameter::type_string:
            {
                auto res = std::make_unique<configuration_parameter_string>(name);
                res->set_value(read_string(is));
                result = std::move(res);
                break;
            }
            case configuration_parameter::type_reference: 
            {
                auto res = std::make_unique<configuration_parameter_reference>(name);
                res->set_target(read_string(is));
                result = std::move(res);
                break;
            }
            case configuration_parameter::type_list: 
            {
                auto list_param = std::make_unique<configuration_parameter_list_sorted>(name);
                uint32_t count;
                read_binary(is, count);

                for (uint32_t i = 0; i < count; ++i)
                    list_param->add(deserialize(is));

                result = std::move(list_param);
                break;
            }
            default:
                return nullptr;
        }

        return result;
    }
}