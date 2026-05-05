#include "controller/registry.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/configuration-item.hpp"
#include "port/input/port-input.hpp"
#include "port/output/port-output.hpp"
#include "data/processor/filter/filter.hpp"
#include "data/processor/converter/converter.hpp"
#include "version/version.hpp"

#include <fstream>
#include <stdexcept>

namespace adam
{
    struct registry_file_header
    {
        static constexpr uint32_t expected_magic = 0xadacf116; // adam ConFig

        uint32_t        magic;
        version_info    version;
    };

    // Helper functions for binary I/O
    template<typename T>
    void write_binary(std::ostream& os, const T& value) 
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template<typename T>
    void read_binary(std::istream& is, T& value) 
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    void write_string(std::ostream& os, const std::string& str) 
    {
        uint32_t len = static_cast<uint32_t>(str.length());
        write_binary(os, len);
        os.write(str.c_str(), len);
    }

    string_hashed read_string(std::istream& is) 
    {
        uint32_t len = 0;
        read_binary(is, len);
        std::string str(len, '\0');
        is.read(&str[0], len);
        return string_hashed(str);
    }

    // Forward declarations for recursive serialization
    void serialize_parameter(std::ostream& os, const configuration_parameter* param);
    std::unique_ptr<configuration_parameter> deserialize_parameter(std::istream& is);

    void serialize_parameter(std::ostream& os, const configuration_parameter* param) 
    {
        if (!param) 
            return;

        write_binary(os, param->get_type());
        write_string(os, param->get_name());

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
                write_string(os, static_cast<const configuration_parameter_reference*>(param)->get_target());
                break;
            case configuration_parameter::list: 
            {
                const auto& children = static_cast<const configuration_parameter_list*>(param)->get_children();
                uint32_t count = static_cast<uint32_t>(children.size());
                write_binary(os, count);

                for (const auto& [child_name, child] : children)
                    serialize_parameter(os, child.get());
                
                break;
            }
            default: break;
        }
    }

    std::unique_ptr<configuration_parameter> deserialize_parameter(std::istream& is) 
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
            {
                return std::make_unique<configuration_parameter_string>(name, read_string(is));
            }
            case configuration_parameter::reference: 
            {
                return std::make_unique<configuration_parameter_reference>(name, read_string(is));
            }
            case configuration_parameter::list: 
            {
                auto list_param = std::make_unique<configuration_parameter_list>(name);
                uint32_t count;
                read_binary(is, count);

                for (uint32_t i = 0; i < count; ++i)
                    list_param->add(deserialize_parameter(is));

                return list_param;
            }
            default:
                return nullptr;
        }
    }

    registry::registry(const controller* ctrl) 
     :  m_general(string_hashed("general")),
        m_input_ports(),
        m_output_ports(),
        m_filters(),
        m_converters(),
        m_controller(ctrl)
    {
        
    }

    registry::~registry() 
    {

    }

    bool registry::save(std::string_view filepath) const 
    {
        std::ofstream ofs(filepath.data(), std::ios::binary);
        if (!ofs) return false;

        registry_file_header header{};
        header.magic = registry_file_header::expected_magic;
        header.version = decode_version(sdk_version);
        write_binary(ofs, header);

        // Mock a root configuration_parameter_list to maintain the file format structure
        write_binary(ofs, configuration_parameter::list);
        write_string(ofs, "root");
        
        uint32_t root_count = 5; // general, input_ports, output_ports, filters, converters
        write_binary(ofs, root_count);

        // 1. Save general settings
        serialize_parameter(ofs, &m_general);

        // Helper lambda to save grouped items into mock lists
        auto serialize_group = [&ofs](const std::string& group_name, const auto& group_map) 
        {
            write_binary(ofs, configuration_parameter::list);
            write_string(ofs, group_name);
            
            uint32_t count = static_cast<uint32_t>(group_map.size());
            write_binary(ofs, count);

            for (const auto& [name, item] : group_map)
                if (item) serialize_parameter(ofs, &item->get_parameters());
        };

        // 2-5. Save grouped configuration items
        serialize_group("input_ports", m_input_ports);
        serialize_group("output_ports", m_output_ports);
        serialize_group("filters", m_filters);
        serialize_group("converters", m_converters);

        return ofs.good();
    }

    bool registry::load(std::string_view filepath) 
    {
        std::ifstream ifs(filepath.data(), std::ios::binary);

        if (!ifs) 
            return false;

        registry_file_header header{};
        read_binary(ifs, header);

        if (header.magic != registry_file_header::expected_magic) 
            return false;

        uint32_t loaded_version = make_version(header.version.major, header.version.minor, header.version.patch);

        if (get_major(loaded_version) > get_major(sdk_version))
            return false;

        auto loaded_root = deserialize_parameter(ifs);

        if (!loaded_root || loaded_root->get_type() != configuration_parameter::list) 
            return false;

        // Clear the existing state
        m_general.clear();
        m_input_ports.clear();
        m_output_ports.clear();
        m_filters.clear();
        m_converters.clear();

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());

        // 1. Restore general settings
        if (auto* general_mock = static_cast<configuration_parameter_list*>(root_list->get(string_hashed("general"))))
        {
            for (auto& [name, param] : general_mock->get_children())
                m_general.add(std::move(param));
        }

        // The tree structure is successfully loaded from the file!
        // In the future, you can implement the item instantiation logic here 
        // by looping through static_cast<configuration_parameter_list*>(loaded_root.get())->get(string_hashed("input_ports"))
        return ifs.good();
    }
}