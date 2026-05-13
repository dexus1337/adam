#include "controller/registry.hpp"

#include "controller/controller.hpp"
#include "controller/controller-module-manager.hpp"
#include "configuration/configuration-item.hpp"
#include "data/port/port-input-internal.hpp"
#include "data/port/port-output-internal.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/connection.hpp"
#include "version/version.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"

#include <fstream>
#include <string>
#include <stdexcept>

namespace adam
{
    static default_factory<port, port_input_internal>   global_port_input_internal_factory  = default_factory<port, port_input_internal>();
    static default_factory<port, port_output_internal>  global_port_output_internal_factory = default_factory<port, port_output_internal>();

    const configuration_parameter_list& registry::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_integer>(string_hashed("language"), language_english));
            return p;
        }();
        return params;
    }

    registry::registry(const controller& ctrl) 
     :  configuration_item(string_hashed("general")),
        m_ports(),
        m_filters(),
        m_converters(),
        m_connections(),
        m_default_port_factory(),
        m_controller(ctrl)
    {
        add_parameters(get_default_parameters());

        // Guarantee the global fast-access pointer is bound even if load() fails due to a missing file.
        if (this == &m_controller.get_registry())
        {
            const_cast<controller&>(m_controller).m_lang_param = static_cast<configuration_parameter_integer*>(m_parameters.get(string_hashed("language")));
        }

        m_default_port_factory.emplace
        (
            port_input_internal::type_name, 
            &global_port_input_internal_factory
        );

        m_default_port_factory.emplace
        (
            port_output_internal::type_name, 
            &global_port_output_internal_factory
        );

        load("adam-config.bin");
    }

    registry::~registry() 
    {
        save("adam-config.bin");
    }

    void registry::clear()
    {
        m_ports.clear();
        m_filters.clear();
        m_converters.clear();
        m_connections.clear();
    }

    port* registry::create_port(const string_hashed& name, const string_hashed& type, const string_hashed& module_name)
    {
        // 1. Ensure the port name is unique across all ports
        if (m_ports.find(name) != m_ports.end())
            return nullptr;

        const factory<port>* port_factory = nullptr;

        // 2. Lookup the appropriate factory
        if (!module_name.empty())
        {
            const module* mod = m_controller.get_modules().get_loaded_module(module_name);
            if (!mod)
                return nullptr; // Module not found or not loaded

            const auto& mod_factories = mod->get_port_factories();
            auto it = mod_factories.find(type);
            if (it != mod_factories.end())
                port_factory = it->second;
        }
        else
        {
            auto it = m_default_port_factory.find(type);
            if (it != m_default_port_factory.end())
                port_factory = it->second;
        }

        if (!port_factory)
            return nullptr; // Unknown port type

        // 3. Create the port and immediately take ownership in the registry
        port* new_port = port_factory->create(name);
        if (new_port)
            m_ports.emplace(name, std::unique_ptr<port>(new_port));
        
        return new_port;
    }

    bool registry::remove_port(const string_hashed& name)
    {
        auto port_it = m_ports.find(name);
        if (port_it == m_ports.end())
            return false;

        port* port_to_remove = port_it->second.get();

        // Remove this port from any connections that might be using it to prevent dangling pointers.
        for (auto& [conn_name, conn_ptr] : m_connections)
        {
            if (conn_ptr)
            {
                // The port could be an input or an output port.
                // We need to try removing it from both lists in the connection.
                if (auto input_port = dynamic_cast<port_input*>(port_to_remove))
                {
                    conn_ptr->ports_input().remove(input_port);
                }
                if (auto output_port = dynamic_cast<port_output*>(port_to_remove))
                {
                    conn_ptr->ports_output().remove(output_port);
                }
            }
        }

        m_ports.erase(port_it);
        return true;
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

        auto copy_parameters = [](configuration_parameter_list* target, configuration_parameter_list* source) 
        {
            if (!target || !source) return;
            for (auto& [name, param] : source->get_children()) 
            {
                if (auto* existing = target->get(name)) 
                {
                    if (auto* e_bool = dynamic_cast<configuration_parameter_boolean*>(existing)) {
                        if (auto* p_bool = dynamic_cast<configuration_parameter_boolean*>(param.get())) e_bool->set_value(p_bool->get_value());
                    } else if (auto* e_int = dynamic_cast<configuration_parameter_integer*>(existing)) {
                        if (auto* p_int = dynamic_cast<configuration_parameter_integer*>(param.get())) e_int->set_value(p_int->get_value());
                    } else if (auto* e_dbl = dynamic_cast<configuration_parameter_double*>(existing)) {
                        if (auto* p_dbl = dynamic_cast<configuration_parameter_double*>(param.get())) e_dbl->set_value(p_dbl->get_value());
                    } else if (auto* e_str = dynamic_cast<configuration_parameter_string*>(existing)) {
                        if (auto* p_str = dynamic_cast<configuration_parameter_string*>(param.get())) e_str->set_value(p_str->get_value());
                    }
                }
            }
        };

        // 1. Restore general settings
        if (auto* general_mock_param = root_list->get(string_hashed("general")))
        {
            if (general_mock_param->get_type() == configuration_parameter::list)
            {
                copy_parameters(&m_parameters, static_cast<configuration_parameter_list*>(general_mock_param));
            }
        }

        if (this == &m_controller.get_registry())
            {
            const_cast<controller&>(m_controller).m_lang_param = static_cast<configuration_parameter_integer*>(m_parameters.get(string_hashed("language")));
            if (!m_controller.m_lang_param)
            {
                auto lang_param = std::make_unique<configuration_parameter_integer>(string_hashed("language"), language_english);
                const_cast<controller&>(m_controller).m_lang_param = lang_param.get();
                m_parameters.add(std::move(lang_param));
            }
        }

        // 2. Restore ports
        if (auto* ports_mock_param = root_list->get(string_hashed("ports")))
        {
            if (ports_mock_param->get_type() == configuration_parameter::list)
            {
                auto* ports_list = static_cast<configuration_parameter_list*>(ports_mock_param);
                for (auto& [port_name, port_param] : ports_list->get_children())
                {
                    if (port_param && port_param->get_type() == configuration_parameter::list)
                    {
                        auto* port_params = static_cast<configuration_parameter_list*>(port_param.get());
                        auto* type_param = port_params->get(string_hashed("type"));
                        if (type_param && type_param->get_type() == configuration_parameter::string)
                        {
                            string_hashed port_type = static_cast<configuration_parameter_string*>(type_param)->get_value();
                            string_hashed module_name;
                            
                            if (auto* mod_param = port_params->get(string_hashed("module_name")))
                            {
                                if (mod_param->get_type() == configuration_parameter::string)
                                    module_name = static_cast<configuration_parameter_string*>(mod_param)->get_value();
                            }

                            port* new_port = create_port(port_name, port_type, module_name);
                            if (new_port)
                            {
                                copy_parameters(&new_port->get_parameters(), port_params);
                            }
                        }
                    }
                }
            }
        }

        return ifs.good();
    }
}