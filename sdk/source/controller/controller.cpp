#include "controller/controller.hpp"

#include <filesystem>
#include <vector>
#include <iostream>

#ifdef   ADAM_PLATFORM_LINUX
#include <dlfcn.h>
#elifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include "module/module.hpp"

namespace adam
{
    controller::controller() {}

    controller::~controller() {}

    const module* controller::get_module(const string_hashed& name) const 
    {
        auto it = m_modules.find(name);

        if (it != m_modules.end()) 
        {
            return it->second;
        }

        return nullptr;
    }

    bool controller::scan_for_modules(std::string_view directory) 
    {
        std::vector<std::filesystem::path> possible_module_paths;

        try 
        {
            if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) 
            {
                // directory_iterator for just this folder
                // recursive_directory_iterator if you want to search subfolders
                for (const auto& entry : std::filesystem::directory_iterator(directory)) 
                {
                    if (!entry.is_regular_file())
                        continue;

                    #ifdef ADAM_PLATFORM_WINDOWS
                    if (entry.path().extension() != ".dll")
                        continue;
                    #else
                    if (entry.path().extension() != ".so")
                        continue;
                    #endif

                    possible_module_paths.push_back(entry.path());
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            return false;
        }

        for (const auto& path : possible_module_paths)
        {
            module* mod         = nullptr;
            auto path_str       = std::string(std::filesystem::absolute(path).string());   
            #ifdef ADAM_PLATFORM_LINUX
            auto handle         = dlopen(path_str.c_str(), RTLD_LAZY);

            if (!handle)
                continue;

            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));

            if (!fn_get_adam_module)
                goto UNLOAD_AND_CONTINUE;
            
            mod = fn_get_adam_module();

            if (!mod)
                goto UNLOAD_AND_CONTINUE;

            if (m_modules.contains(mod->get_name()))
                continue;

            if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
                goto UNLOAD_AND_CONTINUE;

            m_modules.emplace(mod->get_name(), mod);

            mod->m_mod_handle   = reinterpret_cast<uintptr_t>(handle);
            mod->m_str_filepath = string_hashed(path_str);

            continue;

        UNLOAD_AND_CONTINUE:
            dlclose(handle);
            continue;
            #elifdef ADAM_PLATFORM_WINDOWS
            auto handle = LoadLibraryW(std::filesystem::absolute(path).c_str());

            if (!handle)
                continue;

            // GetProcAddress uses LPCSTR (char*), even in Unicode builds
            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));

            if (!fn_get_adam_module)
                goto UNLOAD_AND_CONTINUE;

            mod = fn_get_adam_module();

            if (!mod)
                goto UNLOAD_AND_CONTINUE;

            if (m_modules.contains(mod->get_name()))
                goto UNLOAD_AND_CONTINUE;

            if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
                goto UNLOAD_AND_CONTINUE;

            m_modules.emplace(mod->get_name(), mod);

            continue;

        UNLOAD_AND_CONTINUE:
            FreeLibrary(handle);
            continue;
            #endif 
        }        

        return true;
    }
}