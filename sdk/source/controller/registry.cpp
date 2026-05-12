#include "controller/registry.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/configuration-item.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/connection.hpp"
#include "version/version.hpp"

#include <fstream>
#include <string>
#include <stdexcept>

namespace adam
{
    registry::registry(const controller& ctrl) 
     :  configuration_item(string_hashed("general")),
        m_ports(),
        m_filters(),
        m_converters(),
        m_controller(ctrl)
    {
        auto lang_param = std::make_unique<configuration_parameter_integer>(string_hashed("language"), language_english);
        const_cast<controller&>(m_controller).m_lang_param = lang_param.get();
        m_parameters.add(std::move(lang_param));

        load("adam-config.bin");
    }

    registry::~registry() 
    {
        save("adam-config.bin");
    }

    void registry::clear()
    {
        const_cast<controller&>(m_controller).m_lang_param = nullptr;
        m_ports.clear();
        m_filters.clear();
        m_converters.clear();
        m_connections.clear();
    }

    bool registry::save(string_hashed::view filepath) const 
    {
        std::ofstream ofs(std::string(filepath), std::ios::binary);
        
        if (!ofs) 
            return false;

        uint32_t magic = 0xadacf116;
        configuration_parameter::write_binary(ofs, magic);
        configuration_parameter::write_binary(ofs, decode_version(sdk_version));

        // Mock a root configuration_parameter_list to maintain the file format structure
        configuration_parameter::write_binary(ofs, configuration_parameter::list);
        configuration_parameter::write_string(ofs, "root");
        
        uint32_t root_count = 5; // general, input_ports, output_ports, filters, converters
        configuration_parameter::write_binary(ofs, root_count);

        // 1. Save general settings
        configuration_parameter::serialize(ofs, &m_parameters);

        // Helper lambda to save grouped items into mock lists
        auto serialize_group = [&ofs](const std::string& group_name, const auto& group_map) 
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::list);
            configuration_parameter::write_string(ofs, group_name);
            
            uint32_t count = static_cast<uint32_t>(group_map.size());
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : group_map)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
        };

        // 2-5. Save grouped configuration items
        serialize_group("ports", m_ports);
        serialize_group("filters", m_filters);
        serialize_group("converters", m_converters);
        serialize_group("connections", m_connections);

        return ofs.good();
    }

    bool registry::load(string_hashed::view filepath) 
    {
        std::ifstream ifs(std::string(filepath), std::ios::binary);

        if (!ifs) 
            return false;

        uint32_t magic;
        configuration_parameter::read_binary(ifs, magic);

        if (magic != 0xadacf116) 
            return false;

        version_info ver;
        configuration_parameter::read_binary(ifs, ver);
        uint32_t loaded_version = make_version(ver.major, ver.minor, ver.patch);

        if (get_major(loaded_version) > get_major(sdk_version))
            return false;

        auto loaded_root = configuration_parameter::deserialize(ifs);

        if (!loaded_root || loaded_root->get_type() != configuration_parameter::list) 
            return false;

        // Clear the existing state
        clear();

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());

        // 1. Restore general settings
        if (auto* general_mock_param = root_list->get(string_hashed("general")))
        {
            if (general_mock_param->get_type() == configuration_parameter::list)
            {
                auto* general_mock = static_cast<configuration_parameter_list*>(general_mock_param);
                for (auto& [name, param] : general_mock->get_children())
                {
                    if (auto* existing = m_parameters.get(name))
                    {
                        if (auto* e_bool = dynamic_cast<configuration_parameter_boolean*>(existing)) 
                        {
                            if (auto* p_bool = dynamic_cast<configuration_parameter_boolean*>(param.get())) 
                                e_bool->set_value(p_bool->get_value());
                        } 
                        else if (auto* e_int = dynamic_cast<configuration_parameter_integer*>(existing)) 
                        {
                            if (auto* p_int = dynamic_cast<configuration_parameter_integer*>(param.get())) 
                                e_int->set_value(p_int->get_value());
                        } 
                        else if (auto* e_dbl = dynamic_cast<configuration_parameter_double*>(existing)) 
                        {
                            if (auto* p_dbl = dynamic_cast<configuration_parameter_double*>(param.get())) 
                                e_dbl->set_value(p_dbl->get_value());
                        } 
                        else if (auto* e_str = dynamic_cast<configuration_parameter_string*>(existing)) 
                        {
                            if (auto* p_str = dynamic_cast<configuration_parameter_string*>(param.get())) 
                                e_str->set_value(p_str->get_value());
                        }
                    }
                }
            }
        }

        const_cast<controller&>(m_controller).m_lang_param = static_cast<configuration_parameter_integer*>(m_parameters.get(string_hashed("language")));
        if (!m_controller.m_lang_param)
        {
            auto lang_param = std::make_unique<configuration_parameter_integer>(string_hashed("language"), language_english);
            const_cast<controller&>(m_controller).m_lang_param = lang_param.get();
            m_parameters.add(std::move(lang_param));
        }

        // The tree structure is successfully loaded from the file!
        // In the future, you can implement the item instantiation logic here 
        // by looping through static_cast<configuration_parameter_list*>(loaded_root.get())->get(string_hashed("ports"))
        return ifs.good();
    }
}