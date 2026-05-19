#include "commander/module-view.hpp"
#include "os/os.hpp"
#include "module/module.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "data/port/port.hpp"
#include <mutex>

namespace adam 
{
    void module_view::extract_port_type_and_module(string_hashed::hash_datatype type_hash, string_hashed::hash_datatype module_hash, string_hashed& out_type, string_hashed& out_module) const
    {
        if (type_hash == string_hashed("internal").get_hash())
        {
            out_type = string_hashed("internal");
            out_module = string_hashed("");
            return;
        }

        auto it = m_database.find(module_hash);
        if (it != m_database.end())
        {
            out_module = it->first;
            for (const auto& port : it->second.ports)
            {
                if (port.name_hash == type_hash)
                {
                    if (!port.type_name_str.empty())
                        out_type = string_hashed(port.type_name_str.c_str());
                    return;
                }
            }
        }
    }

    void module_view::extract_datatype_and_module(string_hashed::hash_datatype datatype_hash, string_hashed::hash_datatype module_hash, string_hashed& out_datatype, string_hashed& out_module) const
    {
        if (datatype_hash == string_hashed("transparent").get_hash() || datatype_hash == 0)
        {
            out_datatype = string_hashed("transparent");
            out_module = string_hashed("");
            return;
        }

        auto it = m_database.find(module_hash);
        if (it != m_database.end())
        {
            out_module = it->first;
            for (const auto& fmt : it->second.data_formats)
            {
                if (fmt.get_hash() == datatype_hash)
                {
                    out_datatype = fmt;
                    return;
                }
            }
        }
    }
    
    void module_view::update_module_database(const string_hashed& name, const string_hashed& path, uint32_t version)
    {
        if (m_database.find(name) != m_database.end())
            return; // Already grabbed

        void* handle = os::load_library(path.c_str());
        if (handle)
        {
            auto get_mod = reinterpret_cast<module::get_adam_module_fn>(os::get_library_symbol(handle, module::get_entry_point_name().c_str()));
            if (get_mod)
            {
                module* mod_ptr = get_mod();
                if (mod_ptr)
                {
                    module_info info;
                    info.name = name;
                    info.path = path;
                    info.version = version;
                    
                    for (size_t i = 0; i < languages_count; ++i)
                        info.descriptions[i] = mod_ptr->get_description(static_cast<language>(i));
                        
                    for (const auto& [fmt_name, fmt_ptr] : mod_ptr->get_data_formats())
                        info.data_formats.push_back(fmt_ptr->get_name());
                        
                    for (const auto& [port_name, port_factory] : mod_ptr->get_port_factories())
                    {
                        port_direction dir = port_direction::none;
                        std::string type_name_str;
                        if (port_factory)
                        {
                            if (port* tmp = port_factory->create(string_hashed("temp")))
                            {
                                dir = tmp->get_direction();
                                type_name_str = tmp->get_type_name().c_str();
                                delete tmp;
                            }
                        }
                        info.ports.push_back({port_name, type_name_str, dir});
                    }
                    
                    for (const auto& [flt_name, flt_factory] : mod_ptr->get_filter_factories())
                    {
                        std::string name_str = "Unknown Filter";
                        if (auto* tmp = flt_factory->create(string_hashed("temp_filter")))
                        {
                            if (auto* param = dynamic_cast<configuration_parameter_string*>(tmp->get_parameters().get("type"_ct)))
                            {
                                name_str = param->get_value().c_str();
                            }
                            delete tmp;
                        }
                        info.filters.push_back({flt_name, name_str});
                    }
                        
                    for (const auto& [cnv_name, cnv_factory] : mod_ptr->get_converter_factories())
                    {
                        std::string name_str = "Unknown Converter";
                        if (auto* tmp = cnv_factory->create(string_hashed("temp_converter")))
                        {
                            if (auto* param = dynamic_cast<configuration_parameter_string*>(tmp->get_parameters().get("type"_ct)))
                            {
                                name_str = param->get_value().c_str();
                            }
                            delete tmp;
                        }
                        info.converters.push_back({cnv_name, name_str});
                    }
                        
                    m_database[name] = std::move(info);
                }
            }
            os::unload_library(handle);
        }
    }

    void module_view::load_module(const string_hashed& name, const string_hashed& path, uint32_t version)
    {
        update_module_database(name, path, version);
        m_loaded_modules[name] = std::make_pair(version, path);
    }

    void module_view::unload_module(const string_hashed& name)
    {
        m_loaded_modules.erase(name);
    }

    void module_view::clear()
    {
        std::lock_guard<const module_view> lg(*this);
        m_available_modules.clear();
        m_unavailable_modules.clear();
        m_loaded_modules.clear();
        m_database.clear();
        m_paths.clear();
    }
}