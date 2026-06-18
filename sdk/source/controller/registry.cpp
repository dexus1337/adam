#include "controller/registry.hpp"

#include "controller/controller.hpp"
#include "configuration/configuration-item.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/connection.hpp"
#include "version/version.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
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

    const configuration_parameter_list& registry::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_integer>("language"_ct, language_english));
            
            auto module_paths = std::make_unique<configuration_parameter_list>("module_paths"_ct);
            module_paths->add(std::make_unique<configuration_parameter_string>("0"_ct, "./modules/"_ct));
            p.add(std::move(module_paths));

            p.add(std::move(std::make_unique<configuration_parameter_list>("ports"_ct)));
            p.add(std::move(std::make_unique<configuration_parameter_list_sorted>("processors"_ct)));
            p.add(std::move(std::make_unique<configuration_parameter_list>("connections"_ct)));

            return p;
        }();
        return params;
    }

    registry::registry(controller& ctrl) 
     :  configuration_item("general"_ct),
        m_ports(),
        m_processors(),
        m_connections(),
        m_controller(ctrl),
        m_modules(ctrl)
    {
        add_parameters(get_default_parameters());

        register_internal_module(&internal_module_essential);

        if (!load("adam-config.bin"))
            m_modules.scan_for_modules();
    }

    registry::~registry() 
    {
    }

    const configuration_parameter_list* registry::get_module_paths() const
    {
        return get_parameter<configuration_parameter_list>("module_paths"_ct);
    }

    const module* registry::get_module(string_hash module_hash) const
    {
        auto it = m_modules.get_internal_modules().find(module_hash);
        if (it != m_modules.get_internal_modules().end())
        {
            return it->second;
        }

        auto loaded_it = m_modules.get_loaded_modules().find(module_hash);
        if (loaded_it != m_modules.get_loaded_modules().end())
        {
            return loaded_it->second;
        }

        return nullptr;
    }

    const data_format* registry::get_data_format(string_hash fmt_hash, string_hash mod_hash) const
    {
        const module* mod = get_module(mod_hash);
        if (!mod)
            return nullptr;

        auto fmt_it = mod->get_data_formats().find(fmt_hash);
        if (fmt_it != mod->get_data_formats().end())
            return fmt_it->second;

        return nullptr;
    }
    
    registry::status registry::create_port(const string_hashed& name, string_hash type, string_hash type_module, port** out_port)
    {
        if (out_port) 
            *out_port = nullptr;

        if (m_ports.find(name) != m_ports.end())
            return status_error_port_already_exists;

        const module* mod = get_module(type_module);
        if (!mod)
            return status_error_module_not_found;

        const auto& mod_factories = mod->get_port_factories();
        auto factory_it = mod_factories.find(type);
        if (factory_it == mod_factories.end())
            return status_error_factory_not_found;

        const factory<port>* port_factory = factory_it->second.factory_ptr;
        if (!port_factory)
            return status_error_factory_not_found;

        auto new_port = port_factory->create(name);
        if (!new_port)
            return status_error_creation_failed;

        auto ptr = new_port.get();
        m_ports.emplace(name, std::move(new_port));
        
        if (out_port) 
            *out_port = ptr;
            
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
            auto* inputs_list = conn_ptr->get_parameter<configuration_parameter_list>("inputs"_ct);
            for (auto& [idx_str, param] : inputs_list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() == hash)
                        ref->set_target(new_name);
                }
            }
            auto* outputs_list = conn_ptr->get_parameter<configuration_parameter_list>("outputs"_ct);
            for (auto& [idx_str, param] : outputs_list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() == hash)
                        ref->set_target(new_name);
                }
            }
        }
        return status_success;
    }

    registry::status registry::create_processor(const string_hashed& name, string_hash type, string_hash type_module, processor** out_processor)
    {
        if (out_processor) 
            *out_processor = nullptr;

        if (m_processors.find(name) != m_processors.end())
            return status_error_port_already_exists;

        const module* mod = get_module(type_module);
        if (!mod)
            return status_error_module_not_found;

        const auto& mod_factories = mod->get_processor_factories();
        auto factory_it = mod_factories.find(type);
        if (factory_it == mod_factories.end())
            return status_error_factory_not_found;

        const factory_data_processor& processor_factory = factory_it->second;
        if (!processor_factory.factory_ptr)
            return status_error_factory_not_found;

        auto new_processor = processor_factory.factory_ptr->create(name);
        if (!new_processor)
            return status_error_creation_failed;

        auto ptr = new_processor.get();
        m_processors.emplace(name, std::move(new_processor));
        
        if (out_processor) 
            *out_processor = ptr;
            
        return status_success;
    }

    registry::status registry::destroy_processor(string_hash hash)
    {
        auto processor_it = m_processors.find(hash);
        if (processor_it == m_processors.end())
            return status_error_port_not_found;

        processor* processor_to_remove = processor_it->second.get();

        for (auto& [conn_name, conn_ptr] : m_connections)
        {
            if (conn_ptr)
            {
                conn_ptr->processors().remove(processor_to_remove);
            }
        }

        m_processors.erase(processor_it);
        return status_success;
    }

    registry::status registry::rename_processor(string_hash hash, const string_hashed& new_name)
    {
        auto it = m_processors.find(hash);
        if (it == m_processors.end())
            return status_error_port_not_found;

        if (m_processors.find(new_name.get_hash()) != m_processors.end())
            return status_error_port_already_exists;

        auto proc = std::move(it->second);
        m_processors.erase(it);

        proc->set_name(new_name);
        
        m_processors.emplace(new_name.get_hash(), std::move(proc));

        for (auto& [conn_name, conn_ptr] : m_connections)
        {
            auto* procs_list = conn_ptr->get_parameter<configuration_parameter_list>("processors"_ct);
            for (auto& [idx_str, param] : procs_list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() == hash)
                        ref->set_target(new_name);
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

        new_connection->get_parameter<configuration_parameter_integer>("date_created"_ct)->set_value(static_cast<int64_t>(timestamp));
        new_connection->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(timestamp));
        new_connection->get_parameter<configuration_parameter_integer>("sorting_index"_ct)->set_value(static_cast<int64_t>(m_connections.size()));
        new_connection->get_parameter<configuration_parameter_integer>("color_code"_ct)->set_value(0);

        if (out_connection) 
            *out_connection = new_connection.get();

        m_connections.emplace(name, std::move(new_connection));

        return status_success;
    }

    registry::status registry::destroy_connection(string_hash hash)
    {
        auto it = m_connections.find(hash);
        if (it != m_connections.end())
        {
            it->second->ports_input().iterate([&](const auto& inputs)
            {
                for (auto* p : inputs)
                    p->remove_as_connection_input(it->second.get());
            });

            it->second->ports_output().iterate([&](const auto& outputs)
            {
                for (auto* p : outputs)
                    p->remove_as_connection_output(it->second.get());
            });

            it->second->processors().iterate([&](const auto& processors)
            {
                for (auto* p : processors)
                    p->connections().remove(it->second.get());
            });

            m_connections.erase(it);
            return status_success;
        }

        auto unavail_it = m_unavailable_connections.find(hash);
        if (unavail_it != m_unavailable_connections.end())
        {
            m_unavailable_connections.erase(unavail_it);
            return status_success;
        }

        return status_error_connection_not_found;
    }
    
    registry::status registry::rename_connection(string_hash hash, const string_hashed& new_name)
    {
        auto it = m_connections.find(hash);
        if (it != m_connections.end())
        {
            if (m_connections.find(new_name.get_hash()) != m_connections.end() ||
                m_unavailable_connections.find(new_name.get_hash()) != m_unavailable_connections.end())
                return status_error_connection_already_exists;

            auto conn = std::move(it->second);
            m_connections.erase(it);

            conn->set_name(new_name);
            
            conn->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));
                
            m_connections.emplace(new_name.get_hash(), std::move(conn));
            return status_success;
        }

        auto unavail_it = m_unavailable_connections.find(hash);
        if (unavail_it != m_unavailable_connections.end())
        {
            if (m_connections.find(new_name.get_hash()) != m_connections.end() ||
                m_unavailable_connections.find(new_name.get_hash()) != m_unavailable_connections.end())
                return status_error_connection_already_exists;

            auto conn = std::move(unavail_it->second);
            m_unavailable_connections.erase(unavail_it);

            conn->set_name(new_name);
            
            conn->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));
                
            m_unavailable_connections.emplace(new_name.get_hash(), std::move(conn));
            return status_success;
        }

        return status_error_connection_not_found;
    }

    registry::status registry::connection_add_port(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
        {
            auto unavail_it = m_unavailable_connections.find(conn_hash);
            if (unavail_it == m_unavailable_connections.end())
                return status_error_connection_not_found;

            string_hashed port_name;
            auto port_it = m_ports.find(port_hash);
            if (port_it != m_ports.end())
            {
                port_name = port_it->second->get_name();
            }
            else
            {
                auto unavail_port_it = m_unavailable_ports.find(port_hash);
                if (unavail_port_it != m_unavailable_ports.end())
                    port_name = unavail_port_it->second->get_name();
                else
                    return status_error_port_not_found;
            }

            unavail_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

            auto* list = unavail_it->second->get_parameter<configuration_parameter_list>(is_input ? "inputs"_ct : "outputs"_ct);
            auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(list->get_children().size())));
            param->set_target(port_name);
            list->add(std::move(param));

            return status_success;
        }

        auto port_it = m_ports.find(port_hash);
        if (port_it == m_ports.end())
            return status_error_port_not_found;
            
        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        if (is_input)
        {
            port_it->second->add_as_connection_input(conn_it->second.get());
            conn_it->second->ports_input().push_back(port_it->second.get());
            auto* inputs_list = conn_it->second->get_parameter<configuration_parameter_list>("inputs"_ct);
            auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(inputs_list->get_children().size())));
            param->set_target(port_it->second->get_name());
            inputs_list->add(std::move(param));
        }
        else
        {
            port_it->second->add_as_connection_output(conn_it->second.get());
            conn_it->second->ports_output().push_back(port_it->second.get());
            auto* outputs_list = conn_it->second->get_parameter<configuration_parameter_list>("outputs"_ct);
            auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(outputs_list->get_children().size())));
            param->set_target(port_it->second->get_name());
            outputs_list->add(std::move(param));
        }

        conn_it->second->check_valid_chain();
        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        return status_success;
    }

    registry::status registry::connection_remove_port(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
        {
            auto unavail_it = m_unavailable_connections.find(conn_hash);
            if (unavail_it == m_unavailable_connections.end())
                return status_error_connection_not_found;

            unavail_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

            auto* list = unavail_it->second->get_parameter<configuration_parameter_list>(is_input ? "inputs"_ct : "outputs"_ct);
            std::vector<string_hashed> remaining_ports;

            for (const auto& [idx_str, param] : list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() != port_hash)
                    {
                        remaining_ports.push_back(ref->get_target());
                    }
                }
            }

            list->clear();
            for (size_t i = 0; i < remaining_ports.size(); ++i)
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)));
                param->set_target(remaining_ports[i]);
                list->add(std::move(param));
            }

            return status_success;
        }

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
                port_it->second->remove_as_connection_input(conn_it->second.get());
                conn_it->second->ports_input().remove(port_it->second.get());
                auto* inputs_list = conn_it->second->get_parameter<configuration_parameter_list>("inputs"_ct);
                inputs_list->clear();
                conn_it->second->ports_input().iterate([&](const auto& inputs) 
                {
                    for (size_t i = 0; i < inputs.size(); ++i)
                        inputs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), inputs[i]->get_name()));
                });
            }
            else
            {
                port_it->second->remove_as_connection_output(conn_it->second.get());
                conn_it->second->ports_output().remove(port_it->second.get());
                auto* outputs_list = conn_it->second->get_parameter<configuration_parameter_list>("outputs"_ct);
                outputs_list->clear();
                conn_it->second->ports_output().iterate([&](const auto& outputs) 
                {
                    for (size_t i = 0; i < outputs.size(); ++i)
                        outputs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), outputs[i]->get_name()));
                });
            }
        }
        
        conn_it->second->check_valid_chain();
        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        return status_success;
    }

    registry::status registry::connection_add_processor(string_hash conn_hash, string_hash processor_hash)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
        {
            auto unavail_it = m_unavailable_connections.find(conn_hash);
            if (unavail_it == m_unavailable_connections.end())
                return status_error_connection_not_found;

            string_hashed proc_name;
            auto processor_it = m_processors.find(processor_hash);
            if (processor_it != m_processors.end())
            {
                proc_name = processor_it->second->get_name();
            }
            else
            {
                auto unavail_proc_it = m_unavailable_processors.find(processor_hash);
                if (unavail_proc_it != m_unavailable_processors.end())
                    proc_name = unavail_proc_it->second->get_name();
                else
                    return status_error_port_not_found;
            }

            unavail_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

            auto* list = unavail_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
            auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(list->get_children().size())));
            param->set_target(proc_name);
            list->add(std::move(param));

            return status_success;
        }

        auto processor_it = m_processors.find(processor_hash);
        if (processor_it == m_processors.end())
            return status_error_port_not_found; // Reuse error code or make a new one, but this matches other uses
            
        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        conn_it->second->processors().push_back(processor_it->second.get());
        processor_it->second->connections().push_back(conn_it->second.get());
        auto* procs_list = conn_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
        auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(procs_list->get_children().size())));
        param->set_target(processor_it->second->get_name());
        procs_list->add(std::move(param));

        conn_it->second->check_valid_chain();
        
        return status_success;
    }

    registry::status registry::connection_remove_processor(string_hash conn_hash, string_hash processor_hash)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
        {
            auto unavail_it = m_unavailable_connections.find(conn_hash);
            if (unavail_it == m_unavailable_connections.end())
                return status_error_connection_not_found;

            unavail_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

            auto* list = unavail_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
            std::vector<string_hashed> remaining_procs;

            for (const auto& [idx_str, param] : list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() != processor_hash)
                    {
                        remaining_procs.push_back(ref->get_target());
                    }
                }
            }

            list->clear();
            for (size_t i = 0; i < remaining_procs.size(); ++i)
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)));
                param->set_target(remaining_procs[i]);
                list->add(std::move(param));
            }

            return status_success;
        }

        auto processor_it = m_processors.find(processor_hash);
        if (processor_it == m_processors.end())
        {
            auto& unavail = conn_it->second->unavailable_processors();
            auto it = std::find_if(unavail.begin(), unavail.end(), [processor_hash](const string_hashed& sh) { return sh.get_hash() == processor_hash; });
            if (it != unavail.end())
                unavail.erase(it);
        }
        else
        {
            conn_it->second->processors().remove(processor_it->second.get());
            processor_it->second->connections().remove(conn_it->second.get());
            auto* procs_list = conn_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
            procs_list->clear();
            conn_it->second->processors().iterate([&](const auto& procs) 
            {
                for (size_t i = 0; i < procs.size(); ++i)
                    procs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), procs[i]->get_name()));
            });
        }
            
        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        conn_it->second->check_valid_chain();
        
        return status_success;
    }

    registry::status registry::connection_reorder_processor(string_hash conn_hash, string_hash processor_hash, uint32_t new_index)
    {
        auto conn_it = m_connections.find(conn_hash);
        if (conn_it == m_connections.end())
        {
            auto unavail_it = m_unavailable_connections.find(conn_hash);
            if (unavail_it == m_unavailable_connections.end())
                return status_error_connection_not_found;

            unavail_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

            auto* list = unavail_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
            std::vector<string_hashed> procs;
            string_hashed target_proc;
            bool found = false;

            for (const auto& [idx_str, param] : list->get_children())
            {
                if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                {
                    if (ref->get_target().get_hash() == processor_hash)
                    {
                        target_proc = ref->get_target();
                        found = true;
                    }
                    else
                    {
                        procs.push_back(ref->get_target());
                    }
                }
            }

            if (!found)
                return status_error_port_not_found;

            if (new_index > procs.size())
                new_index = static_cast<uint32_t>(procs.size());

            procs.insert(procs.begin() + new_index, target_proc);

            list->clear();
            for (size_t i = 0; i < procs.size(); ++i)
            {
                auto param = std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)));
                param->set_target(procs[i]);
                list->add(std::move(param));
            }

            return status_success;
        }

        auto processor_it = m_processors.find(processor_hash);
        if (processor_it == m_processors.end())
            return status_error_port_not_found;

        std::vector<processor*> procs;
        conn_it->second->processors().iterate([&](const auto& active_procs)
        {
            procs = active_procs;
        });

        auto it = std::find(procs.begin(), procs.end(), processor_it->second.get());
        if (it == procs.end())
            return status_error_port_not_found;

        procs.erase(it);
        if (new_index > procs.size())
            new_index = static_cast<uint32_t>(procs.size());

        procs.insert(procs.begin() + new_index, processor_it->second.get());

        conn_it->second->processors().reorder(procs);

        auto* procs_list = conn_it->second->get_parameter<configuration_parameter_list>("processors"_ct);
        procs_list->clear();
        for (size_t i = 0; i < procs.size(); ++i)
        {
            procs_list->add(std::make_unique<configuration_parameter_reference>(string_hashed(std::to_string(i)), procs[i]->get_name()));
        }

        conn_it->second->get_parameter<configuration_parameter_integer>("date_edited"_ct)->set_value(static_cast<int64_t>(std::time(nullptr)));

        conn_it->second->check_valid_chain();

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
        configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
        configuration_parameter::write_string(ofs, "root");
        
        uint32_t root_count = 5; // general, ports, processors, connections, modules
        configuration_parameter::write_binary(ofs, root_count);

        // 1. Save general settings
        configuration_parameter::serialize(ofs, &m_parameters);

        // 2. Save ports (combining available and unavailable)
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, "ports");
            
            uint32_t count = 0;
            for (const auto& [name, item] : m_ports) if (item) count++;
            for (const auto& [name, item] : m_unavailable_ports) if (item) count++;
            
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : m_ports)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
                
            for (const auto& [name, item] : m_unavailable_ports)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
        }

        // 3. Save processors (combining available and unavailable)
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, "processors");
            
            uint32_t count = 0;
            for (const auto& [name, item] : m_processors) if (item) count++;
            for (const auto& [name, item] : m_unavailable_processors) if (item) count++;
            
            configuration_parameter::write_binary(ofs, count);

            for (const auto& [name, item] : m_processors)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
                
            for (const auto& [name, item] : m_unavailable_processors)
                if (item) configuration_parameter::serialize(ofs, &item->get_parameters());
        }
        
        {
            configuration_parameter::write_binary(ofs, configuration_parameter::type_list);
            configuration_parameter::write_string(ofs, "connections");
            
            uint32_t count = 0;
            for (const auto& [name, item] : m_connections) if (item) count++;
            for (const auto& [name, item] : m_unavailable_connections) if (item) count++;
            
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

        clear();

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());

        // 1. Restore general settings
        copy_parameters(&m_parameters, static_cast<configuration_parameter_list*>(root_list->get("general"_ct)));

        m_modules.clear_and_unload_all();
        m_modules.scan_for_modules();

        // 2. Restore loaded modules
        if (auto* modules_list = static_cast<configuration_parameter_list*>(root_list->get("modules"_ct)))
        {
            for (auto& [mod_name, mod_param] : modules_list->get_children())
                m_modules.load_module(mod_name);
        }

        // 3. Restore ports
        if (auto* ports_list = static_cast<configuration_parameter_list*>(root_list->get("ports"_ct)))
        {
            for (auto& [port_name, port_param] : ports_list->get_children())
            {
                auto* port_params   = static_cast<configuration_parameter_list*>(port_param.get());
                auto port_type      = static_cast<configuration_parameter_string*>(port_params->get("type"_ct))->get_value().get_hash();
                auto module_name    = static_cast<configuration_parameter_string*>(port_params->get("type_origin_module"_ct))->get_value().get_hash();

                port* new_port = nullptr;
                const status s = create_port(port_name, port_type, module_name, &new_port);

                if (s == status_success)
                {
                    copy_parameters(&new_port->parameters(), port_params);
                }
                else if (s == status_error_module_not_found || s == status_error_factory_not_found)
                {
                    auto upi         = std::make_unique<port::unavailable_info>(port_name);
                    upi->type        = port_type;
                    upi->type_module = module_name;
                    copy_parameters(&upi->parameters(), port_params);
                    m_unavailable_ports[port_name] = std::move(upi);
                }
            }
        }

        // 4. Restore processors
        if (auto* processors_list = static_cast<configuration_parameter_list*>(root_list->get("processors"_ct)))
        {
            for (auto& [processor_name, processor_param] : processors_list->get_children())
            {
                auto* processor_params  = static_cast<configuration_parameter_list*>(processor_param.get());
                auto processor_type     = static_cast<configuration_parameter_string*>(processor_params->get("type"_ct))->get_value().get_hash();
                auto module_name        = static_cast<configuration_parameter_string*>(processor_params->get("type_origin_module"_ct))->get_value().get_hash();
                auto is_filter          = static_cast<configuration_parameter_boolean*>(processor_params->get("is_filter"_ct))->get_value();

                processor* new_processor = nullptr;
                const status s = create_processor(processor_name, processor_type, module_name, &new_processor);

                if (s == status_success)
                {
                    copy_parameters(&new_processor->parameters(), processor_params);
                }
                else if (s == status_error_module_not_found || s == status_error_factory_not_found)
                {
                    auto upi         = std::make_unique<processor::unavailable_info>(processor_name);
                    upi->type        = processor_type;
                    upi->type_module = module_name;
                    upi->is_filter   = is_filter;
                    copy_parameters(&upi->parameters(), processor_params);
                    m_unavailable_processors[processor_name] = std::move(upi);
                }
            }
        }

        // 5. Restore connections
        auto resolve_format = [&](const string_hashed& fmt_name, const string_hashed& mod_name, const data_format*& out_format, bool& missing_module)
        {
            missing_module = false;
            if (!mod_name.empty() && !get_module(mod_name.get_hash()))
            {
                missing_module = true;
            }
            out_format = get_data_format(fmt_name.get_hash(), mod_name.get_hash());
        };

        if (auto* connections_list = static_cast<configuration_parameter_list*>(root_list->get("connections"_ct)))
        {
            for (auto& [conn_name, conn_param] : connections_list->get_children())
            {
                auto* conn_params = static_cast<configuration_parameter_list*>(conn_param.get());

                const auto& in_fmt  = static_cast<configuration_parameter_string*>(conn_params->get("input_format"_ct))->get_value();
                const auto& in_mod  = static_cast<configuration_parameter_string*>(conn_params->get("input_format_module"_ct))->get_value();
                const auto& out_fmt = static_cast<configuration_parameter_string*>(conn_params->get("output_format"_ct))->get_value();
                const auto& out_mod = static_cast<configuration_parameter_string*>(conn_params->get("output_format_module"_ct))->get_value();

                const data_format* resolved_in_fmt  = &data_format_transparent;
                const data_format* resolved_out_fmt = &data_format_transparent;
                bool missing_in  = false;
                bool missing_out = false;

                resolve_format(in_fmt,  in_mod,  resolved_in_fmt,  missing_in);
                resolve_format(out_fmt, out_mod, resolved_out_fmt, missing_out);

                if (missing_in || missing_out)
                {
                    auto uci = std::make_unique<connection::unavailable_info>(conn_name);
                    copy_parameters(&uci->parameters(), conn_params);

                    auto* color = static_cast<configuration_parameter_integer*>(uci->get_parameters().get("color_code"_ct));
                    if (color->get_value() == 0xFFFFFF) color->set_value(0);

                    m_unavailable_connections[conn_name.get_hash()] = std::move(uci);
                    continue;
                }

                connection* new_conn = nullptr;
                if (create_connection(conn_name, &new_conn) != status_success)
                    continue;

                copy_parameters(&new_conn->parameters(), conn_params);

                auto* color = static_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("color_code"_ct));
                if (color->get_value() == 0xFFFFFF) color->set_value(0);

                // inputs
                {
                    auto* inputs_list  = static_cast<configuration_parameter_list*>(new_conn->get_parameters().get("inputs"_ct));
                    const size_t count = inputs_list->get_children().size();
                    for (size_t i = 0; i < count; ++i)
                    {
                        auto* ref       = static_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i))));
                        const auto hash = ref->get_target().get_hash();
                        auto port_it    = m_ports.find(hash);
                        if (port_it != m_ports.end())
                        {
                            new_conn->ports_input().push_back(port_it->second.get());
                            port_it->second->add_as_connection_input(new_conn);
                        }
                        else if (m_unavailable_ports.count(hash))
                        {
                            new_conn->unavailable_inputs().push_back(ref->get_target());
                        }
                    }
                }

                // outputs
                {
                    auto* outputs_list = static_cast<configuration_parameter_list*>(new_conn->get_parameters().get("outputs"_ct));
                    const size_t count = outputs_list->get_children().size();
                    for (size_t i = 0; i < count; ++i)
                    {
                        auto* ref       = static_cast<configuration_parameter_reference*>(outputs_list->get(string_hashed(std::to_string(i))));
                        const auto hash = ref->get_target().get_hash();
                        auto port_it    = m_ports.find(hash);
                        if (port_it != m_ports.end())
                        {
                            new_conn->ports_output().push_back(port_it->second.get());
                            port_it->second->add_as_connection_output(new_conn);
                        }
                        else if (m_unavailable_ports.count(hash))
                        {
                            new_conn->unavailable_outputs().push_back(ref->get_target());
                        }
                    }
                }

                // processors
                {
                    auto* procs_list   = static_cast<configuration_parameter_list*>(new_conn->get_parameters().get("processors"_ct));
                    const size_t count = procs_list->get_children().size();
                    for (size_t i = 0; i < count; ++i)
                    {
                        auto* ref       = static_cast<configuration_parameter_reference*>(procs_list->get(string_hashed(std::to_string(i))));
                        const auto hash = ref->get_target().get_hash();
                        auto proc_it    = m_processors.find(hash);
                        if (proc_it != m_processors.end())
                        {
                            new_conn->processors().push_back(proc_it->second.get());
                            proc_it->second->connections().push_back(new_conn);
                        }
                        else if (m_unavailable_processors.count(hash))
                            new_conn->unavailable_processors().push_back(ref->get_target());
                    }
                }

                // these will also perform the chain checks
                new_conn->set_input_format(resolved_in_fmt);
                new_conn->set_output_format(resolved_out_fmt);
            }
        }
        
        return ifs.good();
    }

    void registry::clear()
    {
        m_ports.clear();
        m_processors.clear();
        m_connections.clear();
        m_unavailable_ports.clear();
        m_unavailable_processors.clear();
        m_unavailable_connections.clear();
    }

    void registry::register_internal_module(const module* mod)
    {
        m_modules.register_internal_module(mod);
    }

    void registry::resume_active_items()
    {
        if (!m_ports.empty() || !m_connections.empty())
        {
            std::vector<response> dummy_responses(1);
            command_context ctx { os::get_current_thread_id(), *this, m_controller, dummy_responses, {}, {}, {} };

            for (auto& [port_hash, port_ptr] : m_ports)
            {
                if (port_ptr->get_parameter<configuration_parameter_boolean>("started"_ct)->get_value())
                {
                    command cmd(command_type::port_start);
                    if (auto* data = cmd.data_as<messages::port_action_data>())
                        data->port = port_hash;
                    m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                }
            }

            for (auto& [conn_hash, conn_ptr] : m_connections)
            {
                if (conn_ptr->get_parameter<configuration_parameter_boolean>("started"_ct)->get_value())
                {
                    command cmd(command_type::connection_start);
                    if (auto* data = cmd.data_as<messages::connection_action_data>())
                        data->connection = conn_hash;
                    m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                }
            }
        }
    }

    void registry::stop_items()
    {
        if (!m_ports.empty() || !m_connections.empty())
        {
            std::vector<response> dummy_responses(1);
            command_context ctx { os::get_current_thread_id(), *this, m_controller, dummy_responses, {}, {}, {} };

            for (auto& [port_hash, port_ptr] : m_ports)
            {
                if (port_ptr->get_parameter<configuration_parameter_boolean>("started"_ct)->get_value())
                {
                    command cmd(command_type::port_stop);
                    if (auto* data = cmd.data_as<messages::port_action_data>())
                        data->port = port_hash;
                    m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                }
            }

            for (auto& [conn_hash, conn_ptr] : m_connections)
            {
                if (conn_ptr->get_parameter<configuration_parameter_boolean>("started"_ct)->get_value())
                {
                    command cmd(command_type::connection_stop);
                    if (auto* data = cmd.data_as<messages::connection_action_data>())
                        data->connection = conn_hash;
                    m_controller.dispatcher().dispatch(&cmd, 1, ctx);
                }
            }
        }
    }

    void registry::retry_unavailable_ports(string_hash module_hash)
    {
        for (auto it = m_unavailable_ports.begin(); it != m_unavailable_ports.end();)
        {
            if (it->second->type_module != module_hash)
            {
                ++it;
                continue;
            }
            
            port* new_port = nullptr;
            status stat = create_port(it->second->get_name(), it->second->type, it->second->type_module, &new_port);
            
            if (stat != status_success || !new_port)
            {
                ++it;
                continue;
            }
                
            copy_parameters(&new_port->parameters(), &it->second->parameters());
                
            for (auto& [conn_hash, conn] : m_connections)
            {
                bool modified = false;
                auto& unavail_in = conn->unavailable_inputs();
                auto in_it = std::find_if(unavail_in.begin(), unavail_in.end(), [&](const string_hashed& sh) 
                {
                    return sh.get_hash() == it->first; 
                });
                    
                if (in_it != unavail_in.end())
                {
                    conn->ports_input().push_back(new_port);
                    new_port->add_as_connection_input(conn.get());
                    unavail_in.erase(in_it);
                    modified = true;
                }

                auto& unavail_out = conn->unavailable_outputs();
                auto out_it = std::find_if(unavail_out.begin(), unavail_out.end(), [&](const string_hashed& sh) 
                {
                    return sh.get_hash() == it->first; 
                });
                    
                if (out_it != unavail_out.end())
                {
                    conn->ports_output().push_back(new_port);
                    new_port->add_as_connection_output(conn.get());
                    unavail_out.erase(out_it);
                    modified = true;
                }

                if (modified)
                    conn->check_valid_chain();
            }
                
            event evt(event_type::port_available);
            auto* evt_data = evt.data_as<port::basic_info>();
            evt_data->setup(new_port->get_name(), it->second->type, it->second->type_module, false, new_port->get_state_buffer()->get_handle());
            evt_data->dir       = new_port->get_direction();
            evt_data->started   = new_port->is_started();
            m_controller.broadcast_event(evt);
            it = m_unavailable_ports.erase(it);
        }
    }

    void registry::mark_ports_unavailable(string_hash module_hash)
    {
        for (auto it = m_ports.begin(); it != m_ports.end();)
        {
            const auto& type_module = it->second->get_parameter<configuration_parameter_string>("type_origin_module"_ct)->get_value();

            if (type_module.get_hash() != module_hash)
            {
                ++it;
                continue;
            }

            auto port_hash = it->first;
            auto port_name = it->second->get_name();

            const auto& orig_type = it->second->get_parameter<configuration_parameter_string>("type"_ct)->get_value();

            auto upi = std::make_unique<port::unavailable_info>(port_name);
            upi->type = orig_type.get_hash();
            upi->type_module = module_hash;
            
            copy_parameters(&upi->parameters(), &it->second->parameters());

            for (auto& [conn_hash, conn] : m_connections)
            {
                bool modified = false;
                auto* inputs_list = conn->get_parameter<configuration_parameter_list>("inputs"_ct);
                for (const auto& [idx_str, param] : inputs_list->get_children())
                {
                    if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                    {
                        if (ref->get_target().get_hash() == port_hash)
                        {
                            conn->ports_input().remove(it->second.get());
                            conn->unavailable_inputs().push_back(ref->get_target());
                            modified = true;
                        }
                    }
                }
                
                auto* outputs_list = conn->get_parameter<configuration_parameter_list>("outputs"_ct);
                for (const auto& [idx_str, param] : outputs_list->get_children())
                {
                    if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                    {
                        if (ref->get_target().get_hash() == port_hash)
                        {
                            conn->ports_output().remove(it->second.get());
                            conn->unavailable_outputs().push_back(ref->get_target());
                            modified = true;
                        }
                    }
                }

                if (modified)
                {
                    if (conn->is_started() && !conn->check_valid_chain())
                    {
                        conn->stop();
                        event evt_stop(event_type::connection_stopped);
                        evt_stop.data_as<messages::connection_action_data>()->connection = conn_hash;
                        m_controller.broadcast_event(evt_stop);
                    }
                }
            }

            event evt(event_type::port_unavailable);
            evt.data_as<messages::port_action_data>()->port = port_hash;
            m_controller.broadcast_event(evt);

            m_unavailable_ports[port_hash] = std::move(upi);
            it = m_ports.erase(it);
        }
    }

    void registry::retry_unavailable_processors(string_hash module_hash)
    {
        for (auto it = m_unavailable_processors.begin(); it != m_unavailable_processors.end();)
        {
            if (it->second->type_module != module_hash)
            {
                ++it;
                continue;
            }

            processor* new_processor = nullptr;
            status stat = create_processor(it->second->get_name(), it->second->type, it->second->type_module, &new_processor);

            if (stat != status_success || !new_processor)
            {
                ++it;
                continue;
            }

            copy_parameters(&new_processor->parameters(), &it->second->parameters());
            
            for (auto& [conn_hash, conn] : m_connections)
            {
                auto& unavail_procs = conn->unavailable_processors();
                auto proc_it = std::find_if(unavail_procs.begin(), unavail_procs.end(), [&](const string_hashed& sh) 
                {
                    return sh.get_hash() == it->first; 
                });
                
                if (proc_it == unavail_procs.end())
                    continue;

                unavail_procs.erase(proc_it);
                new_processor->connections().push_back(conn.get());

                // Rebuild processors list in the correct configuration order
                conn->processors().clear();
                auto* procs_list = conn->get_parameter<configuration_parameter_list_sorted>("processors"_ct);
                if (procs_list)
                {
                    for (auto& [idx_str, param] : procs_list->get_children())
                    {
                        if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                        {
                            auto p_it = m_processors.find(ref->get_target().get_hash());
                            if (p_it != m_processors.end())
                            {
                                conn->processors().push_back(p_it->second.get());
                            }
                        }
                    }
                }

                conn->check_valid_chain();
            }
            
            event evt(event_type::processor_available);
            auto* evt_data = evt.data_as<processor::basic_info>();
            
            evt_data->setup(new_processor->get_name(), it->second->type, it->second->type_module, false, new_processor->get_state_buffer()->get_handle());
            
            auto* in_fmt = new_processor->get_input_format();
            auto* in_mod = in_fmt ? in_fmt->get_origin_module() : nullptr;

            auto* out_fmt = new_processor->get_output_format();
            auto* out_mod = out_fmt ? out_fmt->get_origin_module() : nullptr;

            evt_data->input_datatype            = in_fmt ? in_fmt->get_name().get_hash() : 0ull;
            evt_data->input_datatype_module     = in_mod ? in_mod->get_name().get_hash() : 0ull;

            evt_data->output_datatype           = out_fmt ? out_fmt->get_name().get_hash() : 0ull;
            evt_data->output_datatype_module    = out_mod ? out_mod->get_name().get_hash() : 0ull;

            m_controller.broadcast_event(evt);
            
            // Successfully created, remove from unavailable list
            it = m_unavailable_processors.erase(it);
        }
    }

    void registry::mark_processors_unavailable(string_hash module_hash)
    {
        for (auto it = m_processors.begin(); it != m_processors.end();)
        {
            const auto& type_module = it->second->get_parameter<configuration_parameter_string>("type_origin_module"_ct)->get_value();

            if (type_module.get_hash() != module_hash)
            {
                ++it;
                continue;
            }

            auto processor_hash = it->first;
            auto processor_name = it->second->get_name();

            const auto& orig_type = it->second->get_parameter<configuration_parameter_string>("type"_ct)->get_value();

            auto upi            = std::make_unique<processor::unavailable_info>(processor_name);
            upi->type           = orig_type.get_hash();
            upi->type_module    = module_hash;
            upi->is_filter      = it->second->get_parameter<configuration_parameter_boolean>("is_filter"_ct)->get_value();
            
            copy_parameters(&upi->parameters(), &it->second->parameters());

            for (auto& [conn_hash, conn] : m_connections)
            {
                conn->processors().remove(it->second.get());

                auto* procs_list = conn->get_parameter<configuration_parameter_list>("processors"_ct);
                bool modified = false;
                for (auto& [idx_str, param] : procs_list->get_children())
                {
                    if (auto* ref = dynamic_cast<configuration_parameter_reference*>(param.get()))
                    {
                        if (ref->get_target().get_hash() == processor_hash)
                        {
                            auto& unavail_procs = conn->unavailable_processors();
                            if (std::find_if(unavail_procs.begin(), unavail_procs.end(), [&](const string_hashed& sh) { return sh.get_hash() == processor_hash; }) == unavail_procs.end())
                            {
                                unavail_procs.push_back(ref->get_target());
                                modified = true;
                            }
                        }
                    }
                }

                if (modified)
                {
                    if (conn->is_started() && !conn->check_valid_chain())
                    {
                        conn->stop();
                        event evt_stop(event_type::connection_stopped);
                        evt_stop.data_as<messages::connection_action_data>()->connection = conn_hash;
                        m_controller.broadcast_event(evt_stop);
                    }
                }
            }

            // Notify UI about processor becoming unavailable
            event evt(event_type::processor_unavailable);
            evt.data_as<messages::processor_action_data>()->processor = processor_hash;
            m_controller.broadcast_event(evt);

            m_unavailable_processors[processor_hash] = std::move(upi);
            it = m_processors.erase(it);
        }
    }

    void registry::retry_unavailable_connections(string_hash module_hash)
    {
        for (auto it = m_unavailable_connections.begin(); it != m_unavailable_connections.end();)
        {
            const auto& in_fmt  = it->second->get_parameter<configuration_parameter_string>("input_format"_ct)->get_value();
            const auto& in_mod  = it->second->get_parameter<configuration_parameter_string>("input_format_module"_ct)->get_value();

            const auto& out_fmt = it->second->get_parameter<configuration_parameter_string>("output_format"_ct)->get_value();
            const auto& out_mod = it->second->get_parameter<configuration_parameter_string>("output_format_module"_ct)->get_value();

            bool in_matches = in_mod.get_hash() == module_hash;
            bool out_matches = out_mod.get_hash() == module_hash;

            if (!in_matches && !out_matches)
            {
                ++it;
                continue;
            }

            bool in_missing = !in_mod.empty() && !get_module(in_mod.get_hash());
            bool out_missing = !out_mod.empty() && !get_module(out_mod.get_hash());

            if (in_missing || out_missing)
            {
                ++it;
                continue;
            }

            connection* new_conn = nullptr;
            status stat = create_connection(it->second->get_name(), &new_conn);
            
            if (stat != status_success || !new_conn)
            {
                ++it;
                continue;
            }

            copy_parameters(&new_conn->parameters(), &it->second->parameters());
            
            auto resolve_format = [&](const string_hashed& fmt_name, const string_hashed& mod_name, const data_format*& out_format)
            {
                out_format = get_data_format(fmt_name.get_hash(), mod_name.get_hash());
            };

            const data_format* resolved_in_fmt = &data_format_transparent;
            const data_format* resolved_out_fmt = &data_format_transparent;
            resolve_format(in_fmt, in_mod, resolved_in_fmt);
            resolve_format(out_fmt, out_mod, resolved_out_fmt);

            new_conn->set_input_format(resolved_in_fmt);
            new_conn->set_output_format(resolved_out_fmt);

            auto* inputs_list = new_conn->get_parameter<configuration_parameter_list>("inputs"_ct);
            for (size_t i = 0; i < inputs_list->get_children().size(); ++i)
            {
                if (auto* param = dynamic_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i)))))
                {
                    auto port_hash = param->get_target().get_hash();
                    auto port_it = m_ports.find(port_hash);
                    if (port_it != m_ports.end())
                    {
                        new_conn->ports_input().push_back(port_it->second.get());
                        port_it->second->add_as_connection_input(new_conn);
                    }
                    else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                    {
                        new_conn->unavailable_inputs().push_back(param->get_target());
                    }
                }
            }

            auto* outputs_list = new_conn->get_parameter<configuration_parameter_list>("outputs"_ct);
            for (size_t i = 0; i < outputs_list->get_children().size(); ++i)
            {
                if (auto* param = dynamic_cast<configuration_parameter_reference*>(outputs_list->get(string_hashed(std::to_string(i)))))
                {
                    auto port_hash = param->get_target().get_hash();
                    auto port_it = m_ports.find(port_hash);
                    if (port_it != m_ports.end())
                    {
                        new_conn->ports_output().push_back(port_it->second.get());
                        port_it->second->add_as_connection_output(new_conn);
                    }
                    else if (m_unavailable_ports.find(port_hash) != m_unavailable_ports.end())
                    {
                        new_conn->unavailable_outputs().push_back(param->get_target());
                    }
                }
            }

            auto* procs_list = new_conn->get_parameter<configuration_parameter_list>("processors"_ct);
            for (size_t i = 0; i < procs_list->get_children().size(); ++i)
            {
                if (auto* param = dynamic_cast<configuration_parameter_reference*>(procs_list->get(string_hashed(std::to_string(i)))))
                {
                    auto proc_hash = param->get_target().get_hash();
                    auto proc_it = m_processors.find(proc_hash);
                    if (proc_it != m_processors.end())
                    {
                        new_conn->processors().push_back(proc_it->second.get());
                        proc_it->second->connections().push_back(new_conn);
                    }
                    else if (m_unavailable_processors.find(proc_hash) != m_unavailable_processors.end())
                    {
                        new_conn->unavailable_processors().push_back(param->get_target());
                    }
                }
            }

            event evt(event_type::connection_available);
            auto* evt_data = evt.data_as<connection::basic_info>();
            evt_data->setup(new_conn->get_name());

            evt_data->created       = static_cast<uint64_t>(new_conn->get_parameter<configuration_parameter_integer>("date_created"_ct)->get_value());
            evt_data->edited        = static_cast<uint64_t>(new_conn->get_parameter<configuration_parameter_integer>("date_edited"_ct)->get_value());
            evt_data->sorting_index = static_cast<uint32_t>(new_conn->get_parameter<configuration_parameter_integer>("sorting_index"_ct)->get_value());
            evt_data->color         = static_cast<uint32_t>(new_conn->get_parameter<configuration_parameter_integer>("color_code"_ct)->get_value());

            evt_data->started = new_conn->is_started();
            evt_data->valid_chain = new_conn->check_valid_chain();
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

            for (size_t i = 0; i < new_conn->unavailable_processors().size() && evt_data->processor_count < connection::basic_info::default_type_count; ++i)
                evt_data->processors[evt_data->processor_count++] = new_conn->unavailable_processors()[i].get_hash();

            new_conn->ports_output().iterate([&](const auto& outputs) 
            {
                for (size_t i = 0; i < outputs.size() && evt_data->output_count < connection::basic_info::default_type_count; ++i)
                    evt_data->outputs[evt_data->output_count++] = outputs[i]->get_name().get_hash();
            });

            for (size_t i = 0; i < new_conn->unavailable_outputs().size() && evt_data->output_count < connection::basic_info::default_type_count; ++i)
                evt_data->outputs[evt_data->output_count++] = new_conn->unavailable_outputs()[i].get_hash();

            m_controller.broadcast_event(evt);

            it = m_unavailable_connections.erase(it);
        }
    }

    void registry::mark_connections_unavailable(string_hash module_hash)
    {
        if (module_hash == 0) return;

        for (auto it = m_connections.begin(); it != m_connections.end();)
        {
            const auto& in_mod  = it->second->get_parameter<configuration_parameter_string>("input_format_module"_ct)->get_value();
            const auto& out_mod = it->second->get_parameter<configuration_parameter_string>("output_format_module"_ct)->get_value();

            bool in_matches = in_mod.get_hash() == module_hash;
            bool out_matches = out_mod.get_hash() == module_hash;

            if (!in_matches && !out_matches)
            {
                ++it;
                continue;
            }

            auto conn_hash = it->first;
            auto conn_name = it->second->get_name();

            auto uci = std::make_unique<connection::unavailable_info>(conn_name);
            copy_parameters(&uci->parameters(), &it->second->parameters());

            it->second->ports_input().iterate([&](const auto& inputs)
            {
                for (auto* p : inputs)
                    p->remove_as_connection_input(it->second.get());
            });

            it->second->ports_output().iterate([&](const auto& outputs)
            {
                for (auto* p : outputs)
                    p->remove_as_connection_output(it->second.get());
            });

            it->second->processors().iterate([&](const auto& processors)
            {
                for (auto* p : processors)
                    p->connections().remove(it->second.get());
            });

            event evt(event_type::connection_unavailable);
            evt.data_as<messages::connection_action_data>()->connection = conn_hash;
            m_controller.broadcast_event(evt);

            m_unavailable_connections[conn_hash] = std::move(uci);
            it = m_connections.erase(it);
        }
    }

    bool registry::add_module_path(const string_hashed& path, uint32_t* index)
    {
        auto* list = get_parameter<configuration_parameter_list>("module_paths"_ct);
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
            const auto& old_key = string_hashed(std::to_string(i - 1));
            const auto& new_key = string_hashed(std::to_string(i));
            list->rename_child(old_key, new_key);
        }
        
        auto new_key = string_hashed(std::to_string(target_idx));
        auto new_param = std::make_unique<configuration_parameter_string>(new_key);
        new_param->set_value(path);
        list->add(std::move(new_param));

        return true;
    }

    bool registry::remove_module_path(uint32_t index)
    {
        auto* list = get_parameter<configuration_parameter_list>("module_paths"_ct);
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
            const auto& old_key = string_hashed(std::to_string(i));
            const auto& new_key = string_hashed(std::to_string(i - 1));
            list->rename_child(old_key, new_key);
        }

        return true;
    }
    
    void registry::copy_parameters(configuration_parameter_list* target, configuration_parameter_list* source)
    {
        if (!target || !source) return;

        auto copy_single_parameter = [&](const string_hashed& name, const std::unique_ptr<configuration_parameter>& param) 
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
                        std::unique_ptr<configuration_parameter_list> new_param;
                        if (dynamic_cast<const configuration_parameter_list_sorted*>(param.get()))
                            new_param = std::make_unique<configuration_parameter_list_sorted>(param->get_name());
                        else
                            new_param = std::make_unique<configuration_parameter_list>(param->get_name());

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
        };

            for (const auto& [name, param] : source->get_children())
                copy_single_parameter(name, param);
        }

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
}