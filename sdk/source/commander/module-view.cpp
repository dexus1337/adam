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
    void module_view::extract_port_type_and_module(string_hash type_hash, string_hash module_hash, string_hashed& out_type, string_hashed& out_module) const
    {
        if (type_hash == "internal"_ct.get_hash())
        {
            out_type = "internal"_ct;
            out_module = ""_ct;
            return;
        }

        auto it = m_database.find(module_hash);
        if (it != m_database.end())
        {
            out_module = it->first;
            for (const auto& port : it->second.ports)
            {
                if (port.name.get_hash() == type_hash)
                {
                    out_type = port.name;
                    return;
                }
            }
        }
    }

    void module_view::extract_datatype_and_module(string_hash datatype_hash, string_hash module_hash, string_hashed& out_datatype, string_hashed& out_module) const
    {
        if (datatype_hash == "transparent"_ct.get_hash() || datatype_hash == 0)
        {
            out_datatype = "transparent"_ct;
            out_module = ""_ct;
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

    void module_view::extract_processor_type_and_module(string_hash type_hash, string_hash module_hash, string_hashed& out_type, string_hashed& out_module) const
    {
        auto it = m_database.find(module_hash);
        if (it != m_database.end())
        {
            out_module = it->first;
            for (const auto& proc : it->second.processors)
            {
                if (proc.name.get_hash() == type_hash)
                {
                    out_type = proc.name;
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
                        info.ports.push_back({port_name, port_factory.direction});
                    
                    for (const auto& [proc_name, proc_factory] : mod_ptr->get_processor_factories())
                    {
                        bool is_filter = (proc_factory.input_datatype == proc_factory.output_datatype);
                        info.processors.push_back({proc_name, is_filter});
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

        if (m_loaded_modules.find(name) != m_loaded_modules.end())
            return;

        void* handle = os::load_library(path.c_str());
        if (handle)
        {
            auto get_mod = reinterpret_cast<module::get_adam_module_fn>(os::get_library_symbol(handle, module::get_entry_point_name().c_str()));
            if (get_mod)
            {
                if (module* mod_ptr = get_mod())
                {
                    mod_ptr->m_str_filepath = path;
                    m_loaded_modules[name] = std::make_pair(version, mod_ptr);
                    m_handles[name] = handle;
                    return;
                }
            }
            os::unload_library(handle);
        }
    }

    void module_view::unload_module(const string_hashed& name)
    {
        m_loaded_modules.erase(name);
        auto it = m_handles.find(name);
        if (it != m_handles.end())
        {
            os::unload_library(it->second);
            m_handles.erase(it);
        }
    }

    void module_view::clear()
    {
        std::lock_guard<const module_view> lg(*this);
        for (const auto& [name, handle] : m_handles)
        {
            if (handle)
                os::unload_library(handle);
        }
        m_handles.clear();

        m_available_modules.clear();
        m_unavailable_modules.clear();
        m_loaded_modules.clear();
        m_database.clear();
        m_paths.clear();
    }
}