#include "commander/module-view.hpp"
#include "os/os.hpp"
#include "module/module.hpp"

namespace adam 
{
    void module_view::load_module(const string_hashed& name, const string_hashed& path)
    {
        void* handle = os::load_library(path.c_str());
        module* mod_ptr = nullptr;
        if (handle)
        {
            m_handles[name] = handle;
            auto get_mod = reinterpret_cast<module::get_adam_module_fn>(os::get_library_symbol(handle, module::entry_point_name.c_str()));
            if (get_mod)
            {
                mod_ptr = get_mod();

                if (mod_ptr)
                {
                    mod_ptr->m_mod_handle = reinterpret_cast<uintptr_t>(handle);
                    mod_ptr->m_str_filepath = path;
                }
                
            }
        }
        m_loaded_modules[name] = mod_ptr;
    }

    void module_view::unload_module(const string_hashed& name)
    {
        auto it = m_handles.find(name);
        if (it != m_handles.end())
        {
            if (it->second)
                os::unload_library(it->second);
            m_handles.erase(it);
        }
        m_loaded_modules.erase(name);
    }

    void module_view::clear()
    {
        m_available_modules.clear();
        m_unavailable_modules.clear();
        m_loaded_modules.clear();
        for (auto& pair : m_handles)
        {
            if (pair.second)
                os::unload_library(pair.second);
        }
        m_handles.clear();
        m_paths.clear();
    }
}