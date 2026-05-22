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

        // Guarantee the global fast-access pointer is bound even if load() fails due to a missing file.
        if (this == &m_controller.get_registry())
        {
            const_cast<controller&>(m_controller).m_lang_param = static_cast<configuration_parameter_integer*>(m_parameters.get("language"_ct));
        }

        m_default_port_factory.emplace
        (
            port_internal::type_name(), 
            &global_port_internal_factory
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
        m_unavailable_ports.clear();
    }

    registry::status registry::create_port(const string_hashed& name, string_hash type, string_hash type_module, string_hash format, string_hash format_module, port** out_port)
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

        // 3. Resolve the data format
        if (format == 0)
            format = string_hashed("dataformat_transparent").get_hash();

        const data_format* port_format = nullptr;

        if (format_module != 0)
        {
            auto it = m_modules.get_loaded_modules().find(format_module);
            if (it != m_modules.get_loaded_modules().end())
            {
                const module* mod = it->second;
                auto format_it = mod->get_data_formats().find(format);
                if (format_it != mod->get_data_formats().end())
                    port_format = format_it->second;
            }
        }
        else
        {
            for (const auto& [mod_name, mod] : m_modules.get_loaded_modules())
            {
                auto format_it = mod->get_data_formats().find(format);
                if (format_it != mod->get_data_formats().end())
                {
                    port_format = format_it->second;
                    break;
                }
            }
        }

        // 4. Create the port and immediately take ownership in the registry
        port* new_port = port_factory->create(name);
        if (!new_port)
            return status_error_creation_failed;

        if (type_module != 0)
        {
            if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(new_port->get_parameters().get("type_origin_module"_ct)))
                mod_param->set_value(resolved_module_name);
        }

        if (port_format)
            new_port->set_data_format(port_format);

        m_ports.emplace(name, std::unique_ptr<port>(new_port));
        
        if (out_port) 
            *out_port = new_port;
            
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

    registry::status registry::create_connection(const string_hashed& name, connection** out_connection)
    {
        if (out_connection) 
            *out_connection = nullptr;

        if (m_connections.find(name) != m_connections.end())
            return status_error_connection_already_exists;

        auto new_connection = std::make_unique<connection>(name);
        auto timestamp      = std::time(nullptr);

        auto created_param = std::make_unique<configuration_parameter_integer>("created"_ct);
        created_param->set_value(static_cast<int64_t>(timestamp));
        new_connection->get_parameters().add(std::move(created_param));

        auto edited_param = std::make_unique<configuration_parameter_integer>("edited"_ct);
        edited_param->set_value(static_cast<int64_t>(timestamp));
        new_connection->get_parameters().add(std::move(edited_param));

        auto sorting_param = std::make_unique<configuration_parameter_integer>("sorting_index"_ct);
        sorting_param->set_value(static_cast<int64_t>(m_connections.size()));
        new_connection->get_parameters().add(std::move(sorting_param));

        auto color_param = std::make_unique<configuration_parameter_integer>("color"_ct);
        color_param->set_value(0xFFFFFF);
        new_connection->get_parameters().add(std::move(color_param));

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
        
        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("edited"_ct)))
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
            
        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn_it->second->get_parameters().get("edited"_ct)))
            param->set_value(static_cast<int64_t>(std::time(nullptr)));

        port_it->second->connections().push_back(conn_it->second.get());

        if (is_input)
        {
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
            conn_it->second->ports_output().push_back(port_it->second.get());
            if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(conn_it->second->get_parameters().get("outputs"_ct)))
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(outputs_list->get_children().size())));
                param->set_target(port_it->second->get_name());
                outputs_list->add(std::move(param));
            }
        }
            
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
                        case configuration_parameter::boolean:
                            static_cast<configuration_parameter_boolean*>(existing)->set_value(static_cast<configuration_parameter_boolean*>(param.get())->get_value());
                            break;
                        case configuration_parameter::integer:
                            static_cast<configuration_parameter_integer*>(existing)->set_value(static_cast<configuration_parameter_integer*>(param.get())->get_value());
                            break;
                        case configuration_parameter::double_:
                            static_cast<configuration_parameter_double*>(existing)->set_value(static_cast<configuration_parameter_double*>(param.get())->get_value());
                            break;
                        case configuration_parameter::string:
                            static_cast<configuration_parameter_string*>(existing)->set_value(static_cast<configuration_parameter_string*>(param.get())->get_value());
                            break;
                        case configuration_parameter::list:
                            copy_parameters(static_cast<configuration_parameter_list*>(existing), static_cast<configuration_parameter_list*>(param.get()));
                            break;
                        case configuration_parameter::reference:
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
                    case configuration_parameter::boolean:
                    {
                        auto new_param = std::make_unique<configuration_parameter_boolean>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_boolean*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::integer:
                    {
                        auto new_param = std::make_unique<configuration_parameter_integer>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_integer*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::double_:
                    {
                        auto new_param = std::make_unique<configuration_parameter_double>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_double*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::string:
                    {
                        auto new_param = std::make_unique<configuration_parameter_string>(param->get_name());
                        new_param->set_value(static_cast<configuration_parameter_string*>(param.get())->get_value());
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::list:
                    {
                        auto new_param = std::make_unique<configuration_parameter_list>(param->get_name());
                        copy_parameters(new_param.get(), static_cast<configuration_parameter_list*>(param.get()));
                        target->add(std::move(new_param));
                        break;
                    }
                    case configuration_parameter::reference:
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
        configuration_parameter::write_binary(ofs, configuration_parameter::list);
        configuration_parameter::write_string(ofs, "root");
        
        uint32_t root_count = 6; // general, ports, filters, converters, connections, modules
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

        // 2. Save ports (combining available and unavailable)
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::list);
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
        serialize_group("connections", m_connections);

        // 6. Save loaded modules
        configuration_parameter::write_binary(ofs, configuration_parameter::list);
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

        if (!loaded_root || loaded_root->get_type() != configuration_parameter::list) 
            return false;

        // Clear the existing state
        clear();

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());

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

        // 2. Restore loaded modules
        if (auto* modules_mock_param = root_list->get("modules"_ct))
        {
            if (modules_mock_param->get_type() == configuration_parameter::list)
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
                            
                            if (auto* mod_param = port_params->get("type_origin_module"_ct))
                            {
                                if (mod_param->get_type() == configuration_parameter::string)
                                    module_name = static_cast<configuration_parameter_string*>(mod_param)->get_value();
                            }

                            port* new_port = nullptr;
                            status status = create_port(port_name, port_type.get_hash(), module_name.empty() ? 0 : module_name.get_hash(), 0, 0, &new_port);
                            if (status == status_success && new_port)
                            {
                                copy_parameters(&new_port->get_parameters(), port_params);
                            }
                            else if (status == status_error_module_not_found || status == status_error_factory_not_found)
                            {
                                auto upi = std::make_unique<port::unavailable_info>(port_name);
                                upi->type = port_type.get_hash();
                                upi->type_module = module_name.empty() ? 0 : module_name.get_hash();
                                copy_parameters(&upi->get_parameters(), port_params);
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
            if (connections_mock_param->get_type() == configuration_parameter::list)
            {
                auto* connections_list = static_cast<configuration_parameter_list*>(connections_mock_param);
                for (auto& [conn_name, conn_param] : connections_list->get_children())
                {
                    if (conn_param && conn_param->get_type() == configuration_parameter::list)
                    {
                        auto* conn_params = static_cast<configuration_parameter_list*>(conn_param.get());
                        connection* new_conn = nullptr;
                        status status = create_connection(conn_name, &new_conn);
                        if (status == status_success && new_conn)
                        {
                            copy_parameters(&new_conn->get_parameters(), conn_params);

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
                                        }
                                        else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                                        {
                                            new_conn->unavailable_outputs().push_back(param->get_target());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        

        return ifs.good();
    }

    void registry::retry_unavailable_ports(string_hash module_hash)
    {
        for (auto it = m_unavailable_ports.begin(); it != m_unavailable_ports.end();)
        {
            if (it->second->type_module == module_hash || it->second->type_module == 0)
            {
                port* new_port = nullptr;
                status stat = create_port(it->second->get_name(), it->second->type, it->second->type_module, it->second->format, it->second->format_module, &new_port);
                
                if (stat == status_success && new_port)
                {
                    copy_parameters(&new_port->get_parameters(), &it->second->get_parameters());
                    
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
                            unavail_out.erase(out_it);
                        }
                    }
                    
                    event evt(event_type::port_available);
                    auto* evt_data = evt.data_as<port::basic_info>();
                    evt_data->setup(new_port->get_name(), it->second->type, it->second->type_module, it->second->format, it->second->format_module, false);
                    evt_data->direction = new_port->get_direction();
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

                string_hashed orig_type, orig_format;
                if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("type"_ct)))
                    orig_type = param->get_value();
                if (auto* param = dynamic_cast<configuration_parameter_string*>(it->second->get_parameters().get("data_format"_ct)))
                    orig_format = param->get_value();

                auto upi = std::make_unique<port::unavailable_info>(port_name);
                upi->type = orig_type.get_hash();
                upi->type_module = module_hash;
                upi->format = orig_format.get_hash();
                upi->format_module = 0;
                
                copy_parameters(&upi->get_parameters(), &it->second->get_parameters());

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