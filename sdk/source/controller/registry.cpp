#include "controller/registry.hpp"

#include "controller/controller.hpp"
#include "configuration/configuration-item.hpp"
#include "module/module.hpp"
#include "data/port/port-internal.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/connection.hpp"
#include "version/version.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "commander/messages/event.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/response.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "os/os.hpp"

#include <fstream>
#include <string>
#include <stdexcept>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <ctime>

namespace adam
{
    static default_factory<port, port_internal> global_port_internal_factory = default_factory<port, port_internal>();

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

        m_default_port_factory.emplace
        (
            port_internal::type_name(), 
            &global_port_internal_factory
        );

        if (!load("adam-config.bin"))
            m_modules.scan_for_modules();
    }

    registry::~registry() 
    {
    }

    void registry::clear()
    {
        m_ports.clear();
        m_filters.clear();
        m_converters.clear();
        m_connections.clear();
        m_unavailable_ports.clear();
        m_unavailable_connections.clear();
    }

    registry::status registry::create_port(const string_hashed& name, string_hash type, string_hash type_module, port** out_port)
    {
        if (out_port) 
            *out_port = nullptr;

        // 1. Ensure the port name is unique across all ports
        if (m_ports.find(name) != m_ports.end())
            return status_error_port_already_exists;

        const factory<port>* port_factory = nullptr;
        string_hashed resolved_module_name;

        // 2. Lookup the appropriate factory
        if (type_module != 0)
        {
            const module* mod = nullptr;
            auto it = m_modules.get_loaded_modules().find(type_module);
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
                port_factory = factory_it->second.factory_ptr;
        }
        else
        {
            auto it = m_default_port_factory.find(type);
            if (it != m_default_port_factory.end())
                port_factory = it->second.factory_ptr;
        }

        if (!port_factory)
            return status_error_factory_not_found; // Unknown port type

        // 3. Create the port and immediately take ownership in the registry
        port* new_port = port_factory->create(name);
        if (!new_port)
            return status_error_creation_failed;

        if (type_module != 0)
        {
            if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(new_port->get_parameters().get("type_origin_module"_ct)))
                mod_param->set_value(resolved_module_name);
        }

        m_ports.emplace(name, std::unique_ptr<port>(new_port));
        
        if (out_port) 
            *out_port = new_port;
            
        return status_success;
    }

    registry::status registry::connection_remove_port(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
            return status_error_connection_not_found;

        auto port_it = m_ports.find(port_hash);
        if (port_it == m_ports.end())
        {
            if (is_input)
            {
                auto& unavail = conn_it->second->unavailable_inputs();
                auto it = std::find_if(unavail.begin(), unavail.end(), [port_hash](const string_hashed& sh) { return sh.get_hash() == port_hash; });
                if (it != unavail.end())
                    unavail.erase(it);
            }
            else
            {
                auto& unavail = conn_it->second->unavailable_outputs();
                auto it = std::find_if(unavail.begin(), unavail.end(), [port_hash](const string_hashed& sh) { return sh.get_hash() == port_hash; });
                if (it != unavail.end())
                    unavail.erase(it);
            }
        }
        else
        {
            if (is_input)
            {
                port_it->second->in_connections().remove(conn_it->second.get());
                conn_it->second->ports_input().remove(port_it->second.get());
                if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(conn_it->second->get_parameters().get("inputs"_ct)))
                {
                    inputs_list->clear();
                    conn_it->second->ports_input().iterate([&](const auto& inputs) 
                    {
                        for (size_t i = 0; i < inputs.size(); ++i)
                            inputs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), inputs[i]->get_name()));
                    });
                }
            }
            else
            {
                port_it->second->out_connections().remove(conn_it->second.get());
                conn_it->second->ports_output().remove(port_it->second.get());
                if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(conn_it->second->get_parameters().get("outputs"_ct)))
                {
                    outputs_list->clear();
                    conn_it->second->ports_output().iterate([&](const auto& outputs) 
                    {
                        for (size_t i = 0; i < outputs.size(); ++i)
                            outputs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), outputs[i]->get_name()));
                    });
                }
            }
        }
            
        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn_it->second->get_parameters().get("date_edited"_ct)))
            param->set_value(static_cast<int64_t>(std::time(nullptr)));

        conn_it->second->check_valid_chain();
            
        return status_success;
    }

    registry::status registry::destroy_port(string_hash hash)
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
                conn_ptr->ports_input().remove(port_to_remove);
                conn_ptr->ports_output().remove(port_to_remove);
            }
        }

        m_ports.erase(port_it);
        return status_success;
    }

    registry::status registry::rename_port(string_hash hash, const string_hashed& new_name)
    {
        auto it = m_ports.find(hash);
        if (it == m_ports.end())
            return status_error_port_not_found;

        if (m_ports.find(new_name.get_hash()) != m_ports.end())
            return status_error_port_already_exists;

        auto prt = std::move(it->second);
        m_ports.erase(it);

        prt->set_name(new_name);
        
        m_ports.emplace(new_name.get_hash(), std::move(prt));

        for (auto& [conn_name, conn_ptr] : m_connections)
        {
            if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(conn_ptr->get_parameters().get("inputs"_ct)))
            {
                for (auto& [idx_str, param] : inputs_list->get_children())
                {
                    if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                    {
                        if (ref->get_target().get_hash() == hash)
                            ref->set_target(new_name);
                    }
                }
            }
            if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(conn_ptr->get_parameters().get("outputs"_ct)))
            {
                for (auto& [idx_str, param] : outputs_list->get_children())
                {
                    if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                    {
                        if (ref->get_target().get_hash() == hash)
                            ref->set_target(new_name);
                    }
                }
            }
        }
        return status_success;
    }

    registry::status registry::create_connection(const string_hashed& name, connection** out_connection)
    {
        if (out_connection) 
            *out_connection = nullptr;

        if (m_connections.find(name) != m_connections.end())
            return status_error_connection_already_exists;

        auto new_connection = std::make_unique<connection>(name);
        auto timestamp      = std::time(nullptr);

        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_connection->get_parameters().get("date_created"_ct)))
            param->set_value(static_cast<int64_t>(timestamp));

        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_connection->get_parameters().get("date_edited"_ct)))
            param->set_value(static_cast<int64_t>(timestamp));

        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_connection->get_parameters().get("sorting_index"_ct)))
            param->set_value(static_cast<int64_t>(m_connections.size()));

        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_connection->get_parameters().get("color_code"_ct)))
            param->set_value(0);

        if (out_connection) 
            *out_connection = new_connection.get();

        m_connections.emplace(name, std::move(new_connection));

        return status_success;
    }

    registry::status registry::destroy_connection(string_hash hash)
    {
        auto it = m_connections.find(hash);
        if (it == m_connections.end())
            return status_error_connection_not_found;

        m_connections.erase(it);
        return status_success;
    }
    
    registry::status registry::rename_connection(string_hash hash, const string_hashed& new_name)
    {
        auto it = m_connections.find(hash);
        if (it == m_connections.end())
            return status_error_connection_not_found;

        if (m_connections.find(new_name.get_hash()) != m_connections.end())
            return status_error_connection_already_exists;

        auto conn = std::move(it->second);
        m_connections.erase(it);

        conn->set_name(new_name);
        
        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("date_edited"_ct)))
            param->set_value(static_cast<int64_t>(std::time(nullptr)));
            
        m_connections.emplace(new_name.get_hash(), std::move(conn));
        return status_success;
    }

    registry::status registry::connection_add_port(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
            return status_error_connection_not_found;

        auto port_it = m_ports.find(port_hash);
        if (port_it == m_ports.end())
            return status_error_port_not_found;
            
        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn_it->second->get_parameters().get("date_edited"_ct)))
            param->set_value(static_cast<int64_t>(std::time(nullptr)));

        if (is_input)
        {
            port_it->second->in_connections().push_back(conn_it->second.get());
            conn_it->second->ports_input().push_back(port_it->second.get());
            if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(conn_it->second->get_parameters().get("inputs"_ct)))
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(inputs_list->get_children().size())));
                param->set_target(port_it->second->get_name());
                inputs_list->add(std::move(param));
            }
        }
        else
        {
            port_it->second->out_connections().push_back(conn_it->second.get());
            conn_it->second->ports_output().push_back(port_it->second.get());
            if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(conn_it->second->get_parameters().get("outputs"_ct)))
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(outputs_list->get_children().size())));
                param->set_target(port_it->second->get_name());
                outputs_list->add(std::move(param));
            }
        }

        conn_it->second->check_valid_chain();
            
        return status_success;
    }

    void registry::copy_parameters(configuration_parameter_list* target, configuration_parameter_list* source)
    {
        if (!target || !source) return;
        for (auto& [name, param] : source->get_children()) 
        {
            if (auto* existing = target->get(name)) 
            {
                if (existing->get_type() == param->get_type())
                {
                    switch (existing->get_type())
                    {
                        case configuration_parameter::type_boolean:
                            static_cast<configuration_parameter_boolean*>(existing)->set_value(static_cast<configuration_parameter_boolean*>(param.get())->get_value());
                            break;
                        case configuration_parameter::type_integer:
                            static_cast<configuration_parameter_integer*>(existing)->set_value(static_cast<configuration_parameter_integer*>(param.get())->get_value());
                            break;
                        case configuration_parameter::type_double:
                            static_cast<configuration_parameter_double*>(existing)->set_value(static_cast<configuration_parameter_double*>(param.get())->get_value());
                            break;
                        case configuration_parameter::type_string:
                            static_cast<configuration_parameter_string*>(existing)->set_value(static_cast<configuration_parameter_string*>(param.get())->get_value());
                            break;
                        case configuration_parameter::type_list:
                            copy_parameters(static_cast<configuration_parameter_list*>(existing), static_cast<configuration_parameter_list*>(param.get()));
                            break;
                        case configuration_parameter::type_reference:
                            static_cast<configuration_parameter_reference*>(existing)->set_target(static_cast<configuration_parameter_reference*>(param.get())->get_target());
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                switch (param->get_type())
                {
                    case configuration_parameter::type_boolean:
                    {
                        auto new_param = std::make_unique<configuration_parameter_boolean>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_boolean*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::type_integer:
                    {
                        auto new_param = std::make_unique<configuration_parameter_integer>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_integer*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::type_double:
                    {
                        auto new_param = std::make_unique<configuration_parameter_double>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_double*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::type_string:
                    {
                        auto new_param = std::make_unique<configuration_parameter_string>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_string*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::type_list:
                    {
                        auto new_param = std::make_unique<configuration_parameter_list>(param->get_name());
                        copy_parameters(new_param.get(), static_cast<configuration_parameter_list*>(param.get()));
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::type_reference:
                    {
                        auto new_param = std::make_unique<configuration_parameter_reference>(param->get_name());
                        new_param->set_target(static_cast<configuration_parameter_reference*>(param.get())->get_target());
                        target->add(std::move(new_param));
                        break;
                    }
                    default:
                        break;
                }
            }
        }
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
        configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
        configuration_parameter::write_string(ofs, "root");
        
        uint32_t root_count = 6; // general, ports, filters, converters, connections, modules
        configuration_parameter::write_binary(ofs, root_count);

        // 1. Save general settings
        configuration_parameter::serialize(ofs, &m_parameters);

        // Helper lambda to save grouped items into mock lists
        auto serialize_group = [&ofs](const std::string& group_name, const auto& group_map) 
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, group_name);
            
            uint32_t count = static_cast<uint32_t>(group_map.size());
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : group_map)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
        };

        // 2. Save ports (combining available and unavailable)
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, "ports");
            
            uint32_t count = static_cast<uint32_t>(m_ports.size() + m_unavailable_ports.size());
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : m_ports)
            {
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
            }
                
            for (const auto& [name, item] : m_unavailable_ports)
            {
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
            }
        }

        // 3-5. Save grouped configuration items
        serialize_group("filters", m_filters);
        serialize_group("converters", m_converters);
        
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, "connections");
            
            uint32_t count = static_cast<uint32_t>(m_connections.size() + m_unavailable_connections.size());
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : m_connections)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
                
            for (const auto& [name, item] : m_unavailable_connections)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
        }

        // 6. Save loaded modules
        configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
        configuration_parameter::write_string(ofs, "modules");
        uint32_t mod_count = static_cast<uint32_t>(m_modules.get_loaded_modules().size());
        configuration_parameter::write_binary(ofs, mod_count);
        for (const auto& [name, mod] : m_modules.get_loaded_modules())
        {
            configuration_parameter_list mod_list(name);
            configuration_parameter::serialize(ofs, &mod_list);
        }

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

        if (!loaded_root || loaded_root->get_type() != configuration_parameter::type_list) 
            return false;

        // Clear the existing state
        clear();

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());

        // 1. Restore general settings
        if (auto* general_mock_param = root_list->get("general"_ct))
        {
            if (general_mock_param->get_type() == configuration_parameter::type_list)
            {
                copy_parameters(&m_parameters, static_cast<configuration_parameter_list*>(general_mock_param));
            }
        }

        m_modules.clear_and_unload_all();
        m_modules.scan_for_modules();

        // 2. Restore loaded modules
        if (auto* modules_mock_param = root_list->get("modules"_ct))
        {
            if (modules_mock_param->get_type() == configuration_parameter::type_list)
            {
                auto* modules_list = static_cast<configuration_parameter_list*>(modules_mock_param);
                for (auto& [mod_name, mod_param] : modules_list->get_children())
                {
                    m_modules.load_module(mod_name);
                }
            }
        }

        // 3. Restore ports
        if (auto* ports_mock_param = root_list->get("ports"_ct))
        {
            if (ports_mock_param->get_type() == configuration_parameter::type_list)
            {
                auto* ports_list = static_cast<configuration_parameter_list*>(ports_mock_param);
                for (auto& [port_name, port_param] : ports_list->get_children())
                {
                    if (port_param && port_param->get_type() == configuration_parameter::type_list)
                    {
                        auto* port_params = static_cast<configuration_parameter_list*>(port_param.get());
                        auto* type_param = port_params->get("type"_ct);
                        if (type_param && type_param->get_type() == configuration_parameter::type_string)
                        {
                            string_hashed port_type = static_cast<configuration_parameter_string*>(type_param)->get_value();
                            string_hashed module_name;
                            
                            if (auto* mod_param = port_params->get("type_origin_module"_ct))
                            {
                                if (mod_param->get_type() == configuration_parameter::type_string)
                                    module_name = static_cast<configuration_parameter_string*>(mod_param)->get_value();
                            }

                            port* new_port = nullptr;
                            status status = create_port(port_name, port_type.get_hash(), module_name.empty() ? 0 : module_name.get_hash(), &new_port);
                            if (status == status_success && new_port)
                            {
                                copy_parameters(&new_port->parameters(), port_params);
                            }
                            else if (status == status_error_module_not_found || status == status_error_factory_not_found)
                            {
                                auto upi = std::make_unique<port::unavailable_info>(port_name);
                                upi->type = port_type.get_hash();
                                upi->type_module = module_name.empty() ? 0 : module_name.get_hash();
                                copy_parameters(&upi->parameters(), port_params);
                                m_unavailable_ports[port_name.get_hash()] = std::move(upi);
                            }
                        }
                    }
                }
            }
        }

        // 4. Restore connections
        if (auto* connections_mock_param = root_list->get("connections"_ct))
        {
            if (connections_mock_param->get_type() == configuration_parameter::type_list)
            {
                auto* connections_list = static_cast<configuration_parameter_list*>(connections_mock_param);
                for (auto& [conn_name, conn_param] : connections_list->get_children())
                {
                    if (conn_param && conn_param->get_type() == configuration_parameter::type_list)
                    {
                        auto* conn_params = static_cast<configuration_parameter_list*>(conn_param.get());
                        
                        string_hashed in_fmt, in_mod, out_fmt, out_mod;
                        if (auto* param = dynamic_cast<configuration_parameter_string*>(conn_params->get("input_format"_ct)))
                            in_fmt = param->get_value();
                        if (auto* param = dynamic_cast<configuration_parameter_string*>(conn_params->get("input_format_module"_ct)))
                            in_mod = param->get_value();
                        if (auto* param = dynamic_cast<configuration_parameter_string*>(conn_params->get("output_format"_ct)))
                            out_fmt = param->get_value();
                        if (auto* param = dynamic_cast<configuration_parameter_string*>(conn_params->get("output_format_module"_ct)))
                            out_mod = param->get_value();

                        auto resolve_format = [&](const string_hashed& fmt_name, const string_hashed& mod_name, const data_format*& out_format, bool& missing_module)
                        {
                            missing_module = false;
                            if (fmt_name.empty() || fmt_name == "transparent"_ct)
                            {
                                out_format = &data_format_transparent;
                                return;
                            }

                            if (!mod_name.empty() && mod_name.get_hash() != 0)
                            {
                                auto mod_it = m_modules.get_loaded_modules().find(mod_name.get_hash());
                                if (mod_it != m_modules.get_loaded_modules().end())
                                {
                                    auto fmt_it = mod_it->second->get_data_formats().find(fmt_name.get_hash());
                                    if (fmt_it != mod_it->second->get_data_formats().end())
                                    {
                                        out_format = fmt_it->second;
                                        return;
                                    }
                                }
                                else
                                    missing_module = true;
                            }
                            else
                            {
                                for (const auto& [name, mod] : m_modules.get_loaded_modules())
                                {
                                    auto fmt_it = mod->get_data_formats().find(fmt_name.get_hash());
                                    if (fmt_it != mod->get_data_formats().end())
                                    {
                                        out_format = fmt_it->second;
                                        return;
                                    }
                                }
                            }
                            out_format = &data_format_transparent;
                        };

                        const data_format* resolved_in_fmt = &data_format_transparent;
                        const data_format* resolved_out_fmt = &data_format_transparent;
                        bool missing_in = false;
                        bool missing_out = false;
                        
                        resolve_format(in_fmt, in_mod, resolved_in_fmt, missing_in);
                        resolve_format(out_fmt, out_mod, resolved_out_fmt, missing_out);

                        if (missing_in || missing_out)
                        {
                            auto uci = std::make_unique<connection::unavailable_info>(conn_name);
                            copy_parameters(&uci->parameters(), conn_params);
                            
                            if (auto* param = dynamic_cast<configuration_parameter_integer*>(uci->get_parameters().get("color_code"_ct)))
                            {
                                if (param->get_value() == 0xFFFFFF) param->set_value(0);
                            }

                            m_unavailable_connections[conn_name.get_hash()] = std::move(uci);
                            continue;
                        }

                        connection* new_conn = nullptr;
                        status status = create_connection(conn_name, &new_conn);
                        if (status == status_success && new_conn)
                        {
                            copy_parameters(&new_conn->parameters(), conn_params);
                            
                            if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("color_code"_ct)))
                            {
                                if (param->get_value() == 0xFFFFFF) param->set_value(0);
                            }
                            
                            if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(new_conn->get_parameters().get("inputs"_ct)))
                            {
                                for (size_t i = 0; i < inputs_list->get_children().size(); ++i)
                                {
                                    if (auto* param = dynamic_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i)))))
                                    {
                                        auto port_hash = param->get_target().get_hash();
                                        auto port_it = m_ports.find(port_hash);
                                        if (port_it != m_ports.end())
                                        {
                                            new_conn->ports_input().push_back(port_it->second.get());
                                            port_it->second->in_connections().push_back(new_conn);
                                        }
                                        else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                                        {
                                            new_conn->unavailable_inputs().push_back(param->get_target());
                                        }
                                    }
                                }
                            }

                            if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(new_conn->get_parameters().get("outputs"_ct)))
                            {
                                for (size_t i = 0; i < outputs_list->get_children().size(); ++i)
                                {
                                    if (auto* param = dynamic_cast<configuration_parameter_reference*>(outputs_list->get(string_hashed(std::to_string(i)))))
                                    {
                                        auto port_hash = param->get_target().get_hash();
                                        auto port_it = m_ports.find(port_hash);
                                        if (port_it != m_ports.end())
                                        {
                                            new_conn->ports_output().push_back(port_it->second.get());
                                            port_it->second->out_connections().push_back(new_conn);
                                        }
                                        else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                                        {
                                            new_conn->unavailable_outputs().push_back(param->get_target());
                                        }
                                    }
                                }
                            }

                            // these will also perform the chain checks
                            new_conn->set_input_format(resolved_in_fmt);
                            new_conn->set_output_format(resolved_out_fmt);
                        }
                    }
                }
            }
        }
        

        return ifs.good();
    }

    void registry::resume_active_items()
    {
        if (!m_ports.empty() || !m_connections.empty())
        {
            std::vector<response> dummy_responses(1);
            command_context ctx { os::get_current_thread_id(), *this, m_controller, dummy_responses, {} };

            for (auto& [port_hash, port_ptr] : m_ports)
            {
                if (auto* param = dynamic_cast<configuration_parameter_boolean*>(port_ptr->get_parameters().get("is_active"_ct)))
                {
                    if (param->get_value())
                    {
                        command cmd(command_type::port_start);
                        if (auto* data = cmd.data_as<messages::port_action_data>())
                            data->port = port_hash;
                        m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                    }
                }
            }

            for (auto& [conn_hash, conn_ptr] : m_connections)
            {
                if (auto* param = dynamic_cast<configuration_parameter_boolean*>(conn_ptr->get_parameters().get("is_active"_ct)))
                {
                    if (param->get_value())
                    {
                        command cmd(command_type::connection_start);
                        if (auto* data = cmd.data_as<messages::connection_action_data>())
                            data->connection = conn_hash;
                        m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                    }
                }
            }
        }
    }

    void registry::retry_unavailable_ports(string_hash module_hash)
    {
        for (auto it = m_unavailable_ports.begin(); it != m_unavailable_ports.end();)
        {
            if (it->second->type_module == module_hash || it->second->type_module == 0)
            {
                port* new_port = nullptr;
                status stat = create_port(it->second->get_name(), it->second->type, it->second->type_module, &new_port);
                
                if (stat == status_success && new_port)
                {
                    copy_parameters(&new_port->parameters(), &it->second->parameters());
                    
                    for (auto& [conn_hash, conn] : m_connections)
                    {
                        auto& unavail_in = conn->unavailable_inputs();
                        auto in_it = std::find_if(unavail_in.begin(), unavail_in.end(), [&](const string_hashed& sh) 
                        {
                            return sh.get_hash() == it->first; 
                        });
                        
                        if (in_it != unavail_in.end())
                        {
                            conn->ports_input().push_back(new_port);
                            new_port->in_connections().push_back(conn.get());
                            unavail_in.erase(in_it);
                        }

                        auto& unavail_out = conn->unavailable_outputs();
                        auto out_it = std::find_if(unavail_out.begin(), unavail_out.end(), [&](const string_hashed& sh) 
                        {
                            return sh.get_hash() == it->first; 
                        });
                        
                        if (out_it != unavail_out.end())
                        {
                            conn->ports_output().push_back(new_port);
                            new_port->out_connections().push_back(conn.get());
                            unavail_out.erase(out_it);
                        }
                    }
                    
                    event evt(event_type::port_available);
                    auto* evt_data = evt.data_as<port::basic_info>();
                    evt_data->setup(new_port->get_name(), it->second->type, it->second->type_module, false, new_port->get_state_buffer()->get_handle());

                    evt_data->dir       = new_port->get_direction();
                    evt_data->is_active = new_port->is_active();
                    m_controller.broadcast_event(evt);
                    
                    it = m_unavailable_ports.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    void registry::mark_ports_unavailable(string_hash module_hash)
    {
        for (auto it = m_ports.begin(); it != m_ports.end();)
        {
            string_hashed type_module;
            if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("type_origin_module"_ct)))
            {
                type_module = mod_param->get_value();
            }

            if (type_module.get_hash() == module_hash && module_hash != 0)
            {
                auto port_hash = it->first;
                auto port_name = it->second->get_name();

                string_hashed orig_type;
                if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("type"_ct)))
                    orig_type = param->get_value();

                auto upi = std::make_unique<port::unavailable_info>(port_name);
                upi->type = orig_type.get_hash();
                upi->type_module = module_hash;
                
                copy_parameters(&upi->parameters(), &it->second->parameters());

                for (auto& [conn_hash, conn] : m_connections)
                {
                    if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(conn->get_parameters().get("inputs"_ct)))
                    {
                        for (const auto& [idx_str, param] : inputs_list->get_children())
                        {
                            if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                            {
                                if (ref->get_target().get_hash() == port_hash)
                                {
                                    conn->ports_input().remove(it->second.get());
                                    conn->unavailable_inputs().push_back(ref->get_target());
                                }
                            }
                        }
                    }
                    
                    if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(conn->get_parameters().get("outputs"_ct)))
                    {
                        for (const auto& [idx_str, param] : outputs_list->get_children())
                        {
                            if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                            {
                                if (ref->get_target().get_hash() == port_hash)
                                {
                                    conn->ports_output().remove(it->second.get());
                                    conn->unavailable_outputs().push_back(ref->get_target());
                                }
                            }
                        }
                    }
                }

                event evt(event_type::port_unavailable);
                evt.data_as<messages::port_action_data>()->port = port_hash;
                m_controller.broadcast_event(evt);

                m_unavailable_ports[port_hash] = std::move(upi);
                it = m_ports.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void registry::retry_unavailable_connections(string_hash module_hash)
    {
        for (auto it = m_unavailable_connections.begin(); it != m_unavailable_connections.end();)
        {
            string_hashed in_mod, out_mod;
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("input_format_module"_ct)))
                in_mod = param->get_value();
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("output_format_module"_ct)))
                out_mod = param->get_value();

            string_hashed in_fmt, out_fmt;
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("input_format"_ct)))
                in_fmt = param->get_value();
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("output_format"_ct)))
                out_fmt = param->get_value();
                
            bool is_in_transparent = in_fmt.empty() || in_fmt == "transparent"_ct;
            bool is_out_transparent = out_fmt.empty() || out_fmt == "transparent"_ct;

            bool in_matches = !is_in_transparent && in_mod.get_hash() == module_hash;
            bool out_matches = !is_out_transparent && out_mod.get_hash() == module_hash;

            if (in_matches || out_matches || module_hash == 0 || (is_in_transparent && is_out_transparent))
            {
                bool in_missing = false;
                bool out_missing = false;

                if (!is_in_transparent && in_mod.get_hash() != 0)
                {
                    if (m_modules.get_loaded_modules().find(in_mod.get_hash()) == m_modules.get_loaded_modules().end())
                        in_missing = true;
                }
                if (!is_out_transparent && out_mod.get_hash() != 0)
                {
                    if (m_modules.get_loaded_modules().find(out_mod.get_hash()) == m_modules.get_loaded_modules().end())
                        out_missing = true;
                }

                if (!in_missing && !out_missing)
                {
                    connection* new_conn = nullptr;
                    status stat = create_connection(it->second->get_name(), &new_conn);
                    
                    if (stat == status_success && new_conn)
                    {
                        copy_parameters(&new_conn->parameters(), &it->second->parameters());
                        
                        auto resolve_format = [&](const string_hashed& fmt_name, const string_hashed& mod_name, const data_format*& out_format)
                        {
                            if (fmt_name.empty() || fmt_name == "transparent"_ct)
                            {
                                out_format = &data_format_transparent;
                                return;
                            }

                            if (!mod_name.empty() && mod_name.get_hash() != 0)
                            {
                                auto mod_it = m_modules.get_loaded_modules().find(mod_name.get_hash());
                                if (mod_it != m_modules.get_loaded_modules().end())
                                {
                                    auto fmt_it = mod_it->second->get_data_formats().find(fmt_name.get_hash());
                                    if (fmt_it != mod_it->second->get_data_formats().end())
                                    {
                                        out_format = fmt_it->second;
                                        return;
                                    }
                                }
                            }
                            out_format = &data_format_transparent;
                        };

                        const data_format* resolved_in_fmt = &data_format_transparent;
                        const data_format* resolved_out_fmt = &data_format_transparent;
                        resolve_format(in_fmt, in_mod, resolved_in_fmt);
                        resolve_format(out_fmt, out_mod, resolved_out_fmt);

                        new_conn->set_input_format(resolved_in_fmt);
                        new_conn->set_output_format(resolved_out_fmt);

                        if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(new_conn->get_parameters().get("inputs"_ct)))
                        {
                            for (size_t i = 0; i < inputs_list->get_children().size(); ++i)
                            {
                                if (auto* param = dynamic_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i)))))
                                {
                                    auto port_hash = param->get_target().get_hash();
                                    auto port_it = m_ports.find(port_hash);
                                    if (port_it != m_ports.end())
                                    {
                                        new_conn->ports_input().push_back(port_it->second.get());
                                        port_it->second->in_connections().push_back(new_conn);
                                    }
                                    else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                                    {
                                        new_conn->unavailable_inputs().push_back(param->get_target());
                                    }
                                }
                            }
                        }

                        if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(new_conn->get_parameters().get("outputs"_ct)))
                        {
                            for (size_t i = 0; i < outputs_list->get_children().size(); ++i)
                            {
                                if (auto* param = dynamic_cast<configuration_parameter_reference*>(outputs_list->get(string_hashed(std::to_string(i)))))
                                {
                                    auto port_hash = param->get_target().get_hash();
                                    auto port_it = m_ports.find(port_hash);
                                    if (port_it != m_ports.end())
                                    {
                                        new_conn->ports_output().push_back(port_it->second.get());
                                        port_it->second->out_connections().push_back(new_conn);
                                    }
                                    else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                                    {
                                        new_conn->unavailable_outputs().push_back(param->get_target());
                                    }
                                }
                            }
                        }

                        event evt(event_type::connection_available);
                        auto* evt_data = evt.data_as<connection::basic_info>();
                        evt_data->setup(new_conn->get_name());

                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_created"_ct)))
                            evt_data->created = static_cast<uint64_t>(param->get_value());
                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_edited"_ct)))
                            evt_data->edited = static_cast<uint64_t>(param->get_value());
                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("sorting_index"_ct)))
                            evt_data->sorting_index = static_cast<uint32_t>(param->get_value());
                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("color_code"_ct)))
                            evt_data->color = static_cast<uint32_t>(param->get_value());

                        evt_data->is_active = new_conn->is_active();
                        evt_data->valid_chain = new_conn->is_valid_chain();
                        evt_data->is_unavailable = false;

                        evt_data->input_format = resolved_in_fmt->get_name().get_hash();
                        evt_data->input_format_module = in_mod.get_hash();
                        evt_data->output_format = resolved_out_fmt->get_name().get_hash();
                        evt_data->output_format_module = out_mod.get_hash();

                        evt_data->input_count = 0;
                        evt_data->processor_count = 0;
                        evt_data->output_count = 0;

                        new_conn->ports_input().iterate([&](const auto& inputs) 
                        {
                            for (size_t i = 0; i < inputs.size() && evt_data->input_count < connection::basic_info::default_type_count; ++i)
                                evt_data->inputs[evt_data->input_count++] = inputs[i]->get_name().get_hash();
                        });
                        
                        for (size_t i = 0; i < new_conn->unavailable_inputs().size() && evt_data->input_count < connection::basic_info::default_type_count; ++i)
                            evt_data->inputs[evt_data->input_count++] = new_conn->unavailable_inputs()[i].get_hash();

                        new_conn->processors().iterate([&](const auto& procs) 
                        {
                            evt_data->processor_count = static_cast<uint16_t>(std::min(procs.size(), static_cast<size_t>(connection::basic_info::default_type_count)));
                            for (size_t i = 0; i < evt_data->processor_count; ++i)
                                evt_data->processors[i] = procs[i]->get_name().get_hash();
                        });

                        new_conn->ports_output().iterate([&](const auto& outputs) 
                        {
                            for (size_t i = 0; i < outputs.size() && evt_data->output_count < connection::basic_info::default_type_count; ++i)
                                evt_data->outputs[evt_data->output_count++] = outputs[i]->get_name().get_hash();
                        });

                        for (size_t i = 0; i < new_conn->unavailable_outputs().size() && evt_data->output_count < connection::basic_info::default_type_count; ++i)
                            evt_data->outputs[evt_data->output_count++] = new_conn->unavailable_outputs()[i].get_hash();

                        m_controller.broadcast_event(evt);

                        it = m_unavailable_connections.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
    }

    void registry::mark_connections_unavailable(string_hash module_hash)
    {
        if (module_hash == 0) return;

        for (auto it = m_connections.begin(); it != m_connections.end();)
        {
            string_hashed in_mod, out_mod;
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("input_format_module"_ct)))
                in_mod = param->get_value();
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("output_format_module"_ct)))
                out_mod = param->get_value();

            string_hashed in_fmt, out_fmt;
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("input_format"_ct)))
                in_fmt = param->get_value();
            if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("output_format"_ct)))
                out_fmt = param->get_value();
                
            bool is_in_transparent = in_fmt.empty() || in_fmt == "transparent"_ct;
            bool is_out_transparent = out_fmt.empty() || out_fmt == "transparent"_ct;

            bool in_matches = !is_in_transparent && in_mod.get_hash() == module_hash;
            bool out_matches = !is_out_transparent && out_mod.get_hash() == module_hash;

            if (in_matches || out_matches)
            {
                auto conn_hash = it->first;
                auto conn_name = it->second->get_name();

                auto uci = std::make_unique<connection::unavailable_info>(conn_name);
                copy_parameters(&uci->parameters(), &it->second->parameters());

                it->second->ports_input().iterate([&](const auto& inputs)
                {
                    for (auto* p : inputs)
                        p->in_connections().remove(it->second.get());
                });

                it->second->ports_output().iterate([&](const auto& outputs)
                {
                    for (auto* p : outputs)
                        p->out_connections().remove(it->second.get());
                });

                event evt(event_type::connection_unavailable);
                evt.data_as<messages::connection_action_data>()->connection = conn_hash;
                m_controller.broadcast_event(evt);

                m_unavailable_connections[conn_hash] = std::move(uci);
                it = m_connections.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    const configuration_parameter_list* registry::get_module_paths() const
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