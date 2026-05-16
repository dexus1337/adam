#include "controller/registry.hpp"

#include "controller/controller.hpp"
#include "configuration/configuration-item.hpp"
#include "module/module.hpp"
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
#include <array>
#include <cstdlib>

namespace adam
{
    static default_factory<port, port_input_internal>   global_port_input_internal_factory  = default_factory<port, port_input_internal>();
    static default_factory<port, port_output_internal>  global_port_output_internal_factory = default_factory<port, port_output_internal>();

    std::string_view registry::get_status_text(status status, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            { static_cast<int>(status_success), { "Success.", "Erfolgreich." } },
            { static_cast<int>(status_error_port_already_exists), { "A port with this name already exists.", "Ein Port mit diesem Namen existiert bereits." } },
            { static_cast<int>(status_error_module_not_found), { "The specified module was not found or is not loaded.", "Das angegebene Modul wurde nicht gefunden oder ist nicht geladen." } },
            { static_cast<int>(status_error_factory_not_found), { "No factory found for the specified port type.", "Für den angegebenen Port-Typ wurde keine Factory gefunden." } },
            { static_cast<int>(status_error_port_not_found), { "The specified port was not found.", "Der angegebene Port wurde nicht gefunden." } },
            { static_cast<int>(status_error_creation_failed), { "Port creation failed internally.", "Die interne Erstellung des Ports ist fehlgeschlagen." } },
            { static_cast<int>(status_error_connection_already_exists), { "A connection with this name already exists.", "Eine Verbindung mit diesem Namen existiert bereits." } },
            { static_cast<int>(status_error_connection_not_found), { "The specified connection was not found.", "Die angegebene Verbindung wurde nicht gefunden." } }
        };

        auto it = translations.find(static_cast<int>(status));
        if (it != translations.end()) return it->second[static_cast<size_t>(lang)];
        return "Unknown registry status.";
    }

    const configuration_parameter_list& registry::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_integer>("language"_ct, language_english));
            
            auto module_paths = std::make_unique<configuration_parameter_list>("module_paths"_ct);
            module_paths->add(std::make_unique<configuration_parameter_string>("0"_ct, "./modules/"_ct));
            p.add(std::move(module_paths));

            return p;
        }();
        return params;
    }

    registry::registry(controller& ctrl) 
     :  configuration_item("general"_ct),
        m_ports(),
        m_filters(),
        m_converters(),
        m_connections(),
        m_default_port_factory(),
        m_controller(ctrl),
        m_modules(ctrl)
    {
        add_parameters(get_default_parameters());

        // Guarantee the global fast-access pointer is bound even if load() fails due to a missing file.
        if (this == &m_controller.get_registry())
        {
            const_cast<controller&>(m_controller).m_lang_param = static_cast<configuration_parameter_integer*>(m_parameters.get("language"_ct));
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

    registry::status registry::create_port(const string_hashed& name, string_hashed::hash_datatype type, string_hashed::hash_datatype module_name, port** out_port)
    {
        if (out_port) 
            *out_port = nullptr;

        // 1. Ensure the port name is unique across all ports
        if (m_ports.find(name) != m_ports.end())
            return status_error_port_already_exists;

        const factory<port>* port_factory = nullptr;
        string_hashed resolved_module_name;

        // 2. Lookup the appropriate factory
        if (module_name != 0)
        {
            const module* mod = nullptr;
            auto it = m_modules.get_loaded_modules().find(module_name);
            if (it != m_modules.get_loaded_modules().end())
            {
                mod = it->second;
                resolved_module_name = it->first;
            }

            if (!mod)
                return status_error_module_not_found; // Module not found or not loaded

            const auto& mod_factories = mod->get_port_factories();
            auto factory_it = mod_factories.find(type);
            if (factory_it != mod_factories.end())
                port_factory = factory_it->second;
        }
        else
        {
            auto it = m_default_port_factory.find(type);
            if (it != m_default_port_factory.end())
                port_factory = it->second;
        }

        if (!port_factory)
            return status_error_factory_not_found; // Unknown port type

        // 3. Create the port and immediately take ownership in the registry
        port* new_port = port_factory->create(name);
        if (!new_port)
            return status_error_creation_failed;

        if (module_name != 0)
        {
            if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(new_port->get_parameters().get("module_name"_ct)))
                mod_param->set_value(resolved_module_name);
        }
        m_ports.emplace(name, std::unique_ptr<port>(new_port));
        
        if (out_port) 
            *out_port = new_port;
            
        return status_success;
    }

    registry::status registry::destroy_port(string_hashed::hash_datatype hash)
    {
        auto port_it = m_ports.find(hash);
        if (port_it == m_ports.end())
            return status_error_port_not_found;

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
        return status_success;
    }

    registry::status registry::create_connection(const string_hashed& name, connection** out_connection)
    {
        if (out_connection) 
            *out_connection = nullptr;

        if (m_connections.find(name) != m_connections.end())
            return status_error_connection_already_exists;

        auto new_connection = std::make_unique<connection>(name);
        if (out_connection) 
            *out_connection = new_connection.get();
        m_connections.emplace(name, std::move(new_connection));
        return status_success;
    }

    registry::status registry::destroy_connection(string_hashed::hash_datatype hash)
    {
        auto it = m_connections.find(hash);
        if (it == m_connections.end())
            return status_error_connection_not_found;

        m_connections.erase(it);
        return status_success;
    }
    
    registry::status registry::rename_connection(string_hashed::hash_datatype hash, const string_hashed& new_name)
    {
        auto it = m_connections.find(hash);
        if (it == m_connections.end())
            return status_error_connection_not_found;

        if (m_connections.find(new_name.get_hash()) != m_connections.end())
            return status_error_connection_already_exists;

        auto conn = std::move(it->second);
        m_connections.erase(it);

        conn->set_name(new_name);
        m_connections.emplace(new_name.get_hash(), std::move(conn));
        return status_success;
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
                    if (auto* e_bool = dynamic_cast<configuration_parameter_boolean*>(existing))
                    {
                        if (auto* p_bool = dynamic_cast<configuration_parameter_boolean*>(param.get())) e_bool->set_value(p_bool->get_value());
                    }
                    else if (auto* e_int = dynamic_cast<configuration_parameter_integer*>(existing))
                    {
                        if (auto* p_int = dynamic_cast<configuration_parameter_integer*>(param.get())) e_int->set_value(p_int->get_value());
                    }
                    else if (auto* e_dbl = dynamic_cast<configuration_parameter_double*>(existing))
                    {
                        if (auto* p_dbl = dynamic_cast<configuration_parameter_double*>(param.get())) e_dbl->set_value(p_dbl->get_value());
                    }
                    else if (auto* e_str = dynamic_cast<configuration_parameter_string*>(existing))
                    {
                        if (auto* p_str = dynamic_cast<configuration_parameter_string*>(param.get())) e_str->set_value(p_str->get_value());
                    }
                    else if (auto* e_lst = dynamic_cast<configuration_parameter_list*>(existing))
                    {
                        if (auto* p_lst = dynamic_cast<configuration_parameter_list*>(param.get()))
                        {
                            if (e_lst->get_name() == "module_paths"_ct)
                            {
                                for (auto& [child_name, child_param] : p_lst->get_children())
                                {
                                    if (child_param->get_type() == configuration_parameter::string)
                                    {
                                        if (auto* child_existing = e_lst->get(child_name))
                                        {
                                            if (auto* e_child_str = dynamic_cast<configuration_parameter_string*>(child_existing))
                                            {
                                                e_child_str->set_value(static_cast<configuration_parameter_string*>(child_param.get())->get_value());
                                            }
                                        }
                                        else
                                        {
                                            auto new_param = std::make_unique<configuration_parameter_string>(child_param->get_name());
                                            new_param->set_value(static_cast<configuration_parameter_string*>(child_param.get())->get_value());
                                            e_lst->add(std::move(new_param));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };

        // 1. Restore general settings
        if (auto* general_mock_param = root_list->get("general"_ct))
        {
            if (general_mock_param->get_type() == configuration_parameter::list)
            {
                copy_parameters(&m_parameters, static_cast<configuration_parameter_list*>(general_mock_param));
            }
        }

        m_modules.clear_and_unload_all();
        m_modules.scan_for_modules();

        // 2. Restore ports
        if (auto* ports_mock_param = root_list->get("ports"_ct))
        {
            if (ports_mock_param->get_type() == configuration_parameter::list)
            {
                auto* ports_list = static_cast<configuration_parameter_list*>(ports_mock_param);
                for (auto& [port_name, port_param] : ports_list->get_children())
                {
                    if (port_param && port_param->get_type() == configuration_parameter::list)
                    {
                        auto* port_params = static_cast<configuration_parameter_list*>(port_param.get());
                        auto* type_param = port_params->get("type"_ct);
                        if (type_param && type_param->get_type() == configuration_parameter::string)
                        {
                            string_hashed port_type = static_cast<configuration_parameter_string*>(type_param)->get_value();
                            string_hashed module_name;
                            
                            if (auto* mod_param = port_params->get("module_name"_ct))
                            {
                                if (mod_param->get_type() == configuration_parameter::string)
                                    module_name = static_cast<configuration_parameter_string*>(mod_param)->get_value();
                            }

                            port* new_port = nullptr;
                            status status = create_port(port_name, port_type.get_hash(), module_name.empty() ? 0 : module_name.get_hash(), &new_port);
                            if (status == status_success && new_port)
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

    configuration_parameter_list* registry::get_module_paths() const
    {
        return dynamic_cast<configuration_parameter_list*>(m_parameters.get("module_paths"_ct));
    }

    bool registry::add_module_path(const string_hashed& path, uint32_t* index)
    {
        if (auto* list = dynamic_cast<configuration_parameter_list*>(m_parameters.get("module_paths"_ct)))
        {
            uint32_t max_idx = 0;
            for (const auto& [name, param] : list->get_children())
            {
                if (auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get()))
                {
                    if (str_param->get_value() == path) return false; // Already exists
                }
                uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);
                if (idx >= max_idx) max_idx = idx + 1;
            }
            
            uint32_t target_idx = max_idx;
            if (index && *index > 0 && *index < max_idx)
            {
                target_idx = *index;
            }
            
            if (index)
            {
                *index = target_idx;
            }
            
            for (uint32_t i = max_idx; i > target_idx; --i)
            {
                string_hashed old_key(std::to_string(i - 1));
                string_hashed new_key(std::to_string(i));
                list->rename_child(old_key, new_key);
            }
            
            string_hashed new_key(std::to_string(target_idx));
            auto new_param = std::make_unique<configuration_parameter_string>(new_key);
            new_param->set_value(path);
            list->add(std::move(new_param));

            return true;
        }
        return false;
    }

    bool registry::remove_module_path(uint32_t index)
    {
        if (auto* list = dynamic_cast<configuration_parameter_list*>(m_parameters.get("module_paths"_ct)))
        {
            uint32_t max_idx = 0;
            
            for (const auto& [name, param] : list->get_children())
            {
                uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);
                if (idx >= max_idx) max_idx = idx + 1;
            }

            if (index == 0 || index >= max_idx) return false; // Deny removal of the default (first) module path

            list->remove(string_hashed(std::to_string(index)));
            
            for (uint32_t i = index + 1; i < max_idx; ++i)
            {
                string_hashed old_key(std::to_string(i));
                string_hashed new_key(std::to_string(i - 1));
                list->rename_child(old_key, new_key);
            }

            return true;
        }
        return false;
    }
}