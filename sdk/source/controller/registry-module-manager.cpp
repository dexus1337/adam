#include "controller/registry-module-manager.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "module/module.hpp"
#include "version/version.hpp"
#include "resources/language-strings.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

#include <array>
#include <filesystem>
#include <vector>
#include <format>
#include <cstring>

#ifdef   ADAM_PLATFORM_LINUX
#include <dlfcn.h>
#elifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam
{
    registry_module_manager::registry_module_manager(controller& ctrl) : m_controller(ctrl) {}

    registry_module_manager::~registry_module_manager() 
    {
        clear_and_unload_all();
    }

    std::string_view registry_module_manager::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            {
                static_cast<int>(log_event::module_requires_newer_sdk),
                { "Module \"{}\" requires SDK {:d}.{:d}.{:d}, this is {:d}.{:d}.{:d}", "Modul \"{}\" benötigt SDK {:d}.{:d}.{:d}, dies ist {:d}.{:d}.{:d}" }
            },
            {
                static_cast<int>(log_event::module_requires_newer_sdk_cannot_load),
                { "Module \"{}\" requires SDK {:d}.{:d}.{:d}, this is {:d}.{:d}.{:d}. Cannot be loaded!", "Modul \"{}\" benötigt SDK {:d}.{:d}.{:d}, dies ist {:d}.{:d}.{:d}. Kann nicht geladen werden!" }
            },
            {
                static_cast<int>(log_event::module_available),
                { "Available module: {} ver {:d}.{:d}.{:d} -> ({})", "Verfügbares Modul: {} Ver {:d}.{:d}.{:d} -> ({})" }
            },
            {
                static_cast<int>(log_event::module_loaded),
                { "Loaded module \"{}\" at 0x{:x}", "Modul \"{}\" geladen an 0x{:x}" }
            },
            {
                static_cast<int>(log_event::module_load_failed),
                { "Failed to load module \"{}\"", "Modul \"{}\" konnte nicht geladen werden" }
            },
            {
                static_cast<int>(log_event::module_unloaded),
                { "Unloaded module \"{}\"", "Modul \"{}\" entladen" }
            },
            {
                static_cast<int>(log_event::module_unload_failed),
                { "Failed to unload module \"{}\"", "Modul \"{}\" konnte nicht entladen werden" }
            },
            {
                static_cast<int>(log_event::module_removed),
                { "Removed module \"{}\"", "Modul \"{}\" entfernt" }
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message("registry_module_manager::log_event", val, lang);
    }

    const module* registry_module_manager::get_loaded_module(const string_hashed& name) const 
    {
        auto it = m_loaded_modules.find(name);

        if (it != m_loaded_modules.end()) 
            return it->second;

        return nullptr;
    }

    bool registry_module_manager::scan_for_modules() 
    {
        bool any_success = false;

        auto* module_paths = m_controller.get_registry().get_module_paths();
        if (!module_paths)
            return false;

        std::vector<std::filesystem::path> possible_module_paths;
        std::vector<string_hashed> valid_file_paths;

        for (const auto& [name, param] : module_paths->get_children())
        {
            auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get());
            if (!str_param) continue;

            const auto& directory = str_param->get_value();
            if (directory.empty())
                continue;

            try 
            {
                if (std::filesystem::exists(directory.c_str()) && std::filesystem::is_directory(directory.c_str())) 
                {
                    for (const auto& entry : std::filesystem::directory_iterator(directory.c_str())) 
                    {
                        if (!entry.is_regular_file()) continue;

                        #ifdef ADAM_PLATFORM_WINDOWS
                        if (entry.path().extension() != ".dll") continue;
                        #else
                        if (entry.path().extension() != ".so") continue;
                        #endif

                        possible_module_paths.push_back(entry.path());
                        valid_file_paths.push_back(string_hashed(entry.path().string()));
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e) { continue; }
        }

        auto is_valid_path = [&](const string_hashed& path) 
        {
            for (const auto& vp : valid_file_paths) {
                if (vp == path) return true;
            }
            return false;
        };

        // 1. Unload and remove loaded modules
        for (auto it = m_loaded_modules.begin(); it != m_loaded_modules.end();)
        {
            if (!is_valid_path(it->second->get_filepath()))
            {
                string_hashed name = it->first;
                string_hashed path_str = it->second->get_filepath();
                uint32_t version = it->second->get_version();
                uintptr_t handle = it->second->get_module_handle();

                #ifdef ADAM_PLATFORM_LINUX
                dlclose(reinterpret_cast<void*>(handle));
                #elifdef ADAM_PLATFORM_WINDOWS
                FreeLibrary(reinterpret_cast<HMODULE>(handle));
                #endif

                it = m_loaded_modules.erase(it);

                m_controller.log(log::info, get_log_event_text(log_event::module_removed, m_controller.get_language()), name.c_str());

                event evt(event_type::module_removed);
                auto* mod_info = evt.data_as<module::basic_info>();
                mod_info->setup(module::basic_info::loaded, name.c_str(), path_str.c_str(), version);
                m_controller.broadcast_event(evt);
                
                any_success = true;
            }
            else
            {
                ++it;
            }
        }
        
        // 2. Remove from available modules
        for (auto it = m_available_modules.begin(); it != m_available_modules.end();)
        {
            if (!is_valid_path(it->second.second))
            {
                string_hashed name = it->first;
                string_hashed path_str = it->second.second;
                uint32_t version = it->second.first;

                it = m_available_modules.erase(it);

                m_controller.log(log::info, get_log_event_text(log_event::module_removed, m_controller.get_language()), name.c_str());

                event evt(event_type::module_removed);
                auto* mod_info = evt.data_as<module::basic_info>();
                mod_info->setup(module::basic_info::available, name.c_str(), path_str.c_str(), version);
                m_controller.broadcast_event(evt);
                
                any_success = true;
            }
            else
            {
                ++it;
            }
        }

        // 3. Remove from unavailable modules
        for (auto it = m_unavailable_modules.begin(); it != m_unavailable_modules.end();)
        {
            if (!is_valid_path(std::get<1>(it->second)))
            {
                string_hashed name = it->first;
                string_hashed path_str = std::get<1>(it->second);
                uint32_t version = std::get<0>(it->second);

                it = m_unavailable_modules.erase(it);

                m_controller.log(log::info, get_log_event_text(log_event::module_removed, m_controller.get_language()), name.c_str());

                event evt(event_type::module_removed);
                auto* mod_info = evt.data_as<module::basic_info>();
                mod_info->setup(module::basic_info::unavailable, name.c_str(), path_str.c_str(), version);
                m_controller.broadcast_event(evt);
                
                any_success = true;
            }
            else
            {
                ++it;
            }
        }

        for (const auto& path : possible_module_paths)
        {
            module* mod         = nullptr;
            auto path_str       = string_hashed(path.string());

            #ifdef ADAM_PLATFORM_LINUX
            auto handle = dlopen(path_str.c_str(), RTLD_LAZY);
            if (!handle) 
                continue;

            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name.c_str()));
            #elifdef ADAM_PLATFORM_WINDOWS
            auto handle = LoadLibraryW(std::filesystem::absolute(path).c_str());
            if (!handle) 
                continue;

            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name.c_str()));
            #endif

            mod = fn_get_adam_module();
            if (!mod) 
                goto UNLOAD_AND_CONTINUE;

            if (m_loaded_modules.contains(mod->get_name()) ||
                m_available_modules.contains(mod->get_name()) ||
                m_unavailable_modules.contains(mod->get_name())) 
            {
                goto UNLOAD_AND_CONTINUE;
            }

            if (mod->get_required_sdk_version() > adam::sdk_version)
            {
                auto req_maj = adam::get_major(mod->get_required_sdk_version());
                auto req_min = adam::get_minor(mod->get_required_sdk_version());
                auto req_pat = adam::get_patch(mod->get_required_sdk_version());
                auto sdk_maj = adam::get_major(adam::sdk_version);
                auto sdk_min = adam::get_minor(adam::sdk_version);
                auto sdk_pat = adam::get_patch(adam::sdk_version);

                m_controller.log(log::warning, get_log_event_text(log_event::module_requires_newer_sdk, m_controller.get_language()),
                    path_str.c_str(), req_maj, req_min, req_pat, sdk_maj, sdk_min, sdk_pat);

                m_unavailable_modules.emplace(mod->get_name(), std::make_tuple(mod->get_version(), path_str, module::basic_info::incompat_reason_sdk_too_old));

                event evt(event_type::module_unavailable);
                auto* mod_info = evt.data_as<module::basic_info>();
                mod_info->setup(module::basic_info::unavailable, mod->get_name().c_str(), path_str.c_str(), mod->get_version());
                m_controller.broadcast_event(evt);
                goto UNLOAD_AND_CONTINUE;
            }

            m_available_modules.emplace(mod->get_name(), std::make_pair(mod->get_version(), path_str));
            
            {
                auto ver_maj = adam::get_major(mod->get_version());
                auto ver_min = adam::get_minor(mod->get_version());
                auto ver_pat = adam::get_patch(mod->get_version());
                m_controller.log(log::info, get_log_event_text(log_event::module_available, m_controller.get_language()), mod->get_name().c_str(), ver_maj, ver_min, ver_pat, path_str.c_str());

                event evt(event_type::module_available);
                auto* mod_info = evt.data_as<module::basic_info>();
                mod_info->setup(module::basic_info::available, mod->get_name().c_str(), path_str.c_str(), mod->get_version());
                m_controller.broadcast_event(evt);
            }

            goto UNLOAD_AND_CONTINUE; // Ensure handle is closed after checking, avoiding leaks

        UNLOAD_AND_CONTINUE:
            #ifdef ADAM_PLATFORM_LINUX
            dlclose(handle);
            #elifdef ADAM_PLATFORM_WINDOWS
            FreeLibrary(handle);
            #endif
            
            any_success = true;
            continue;
        }
        return any_success;
    }

    bool registry_module_manager::load_module(const string_hashed& name, const module** out_module)
    {
        auto it = m_available_modules.find(name);
        if (it == m_available_modules.end()) 
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_load_failed, m_controller.get_language()), name.c_str());
            return false;
        }

        const auto& path_str = it->second.second;
        module* mod          = nullptr;

        #ifdef ADAM_PLATFORM_LINUX
        auto handle = dlopen(path_str.c_str(), RTLD_LAZY);
        if (!handle) 
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_load_failed, m_controller.get_language()), name.c_str());
            return false;
        }
        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name.c_str()));
        #elifdef ADAM_PLATFORM_WINDOWS
        auto handle = LoadLibraryA(path_str.c_str());
        if (!handle) 
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_load_failed, m_controller.get_language()), name.c_str());
            return false;
        }
        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name.c_str()));
        #endif

        if (!fn_get_adam_module) goto UNLOAD_AND_RETURN;
        mod = fn_get_adam_module();
        if (!mod || m_loaded_modules.contains(mod->get_name())) goto UNLOAD_AND_RETURN;

        if (mod->get_required_sdk_version() > adam::sdk_version)
        {
            auto req_maj = adam::get_major(mod->get_required_sdk_version());
            auto req_min = adam::get_minor(mod->get_required_sdk_version());
            auto req_pat = adam::get_patch(mod->get_required_sdk_version());
            auto sdk_maj = adam::get_major(adam::sdk_version);
            auto sdk_min = adam::get_minor(adam::sdk_version);
            auto sdk_pat = adam::get_patch(adam::sdk_version);

            m_controller.log(log::error, get_log_event_text(log_event::module_requires_newer_sdk_cannot_load, m_controller.get_language()),
                path_str.c_str(), req_maj, req_min, req_pat, sdk_maj, sdk_min, sdk_pat);
            return false;
        }

        mod->m_mod_handle   = reinterpret_cast<uintptr_t>(handle);
        mod->m_str_filepath = path_str;
        m_loaded_modules.emplace(mod->get_name(), mod);
        if (out_module) *out_module = mod;
        
        m_controller.log(log::info, get_log_event_text(log_event::module_loaded, m_controller.get_language()), name.c_str(), reinterpret_cast<uintptr_t>(handle));

        {
            event evt(event_type::module_loaded);
            auto* mod_info = evt.data_as<module::basic_info>();
            mod_info->setup(module::basic_info::loaded, mod->get_name().c_str(), path_str.c_str(), mod->get_version());
            m_controller.broadcast_event(evt);
        }
        return true;

    UNLOAD_AND_RETURN:
        m_controller.log(log::error, get_log_event_text(log_event::module_load_failed, m_controller.get_language()), name.c_str());
        #ifdef ADAM_PLATFORM_LINUX
        if (handle) dlclose(handle);
        #elifdef ADAM_PLATFORM_WINDOWS
        if (handle) FreeLibrary(handle);
        #endif
        return false;
    }

    bool registry_module_manager::unload_module(const string_hashed& name)
    {
        auto it = m_loaded_modules.find(name);
        if (it == m_loaded_modules.end()) 
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_unload_failed, m_controller.get_language()), name.c_str());
            return false;
        }

        const module* mod = it->second;
        uint32_t version = mod->get_version();
        string_hashed path_str = mod->get_filepath();
        uintptr_t handle = mod->get_module_handle();

        #ifdef ADAM_PLATFORM_LINUX
        if (dlclose(reinterpret_cast<void*>(handle)) != 0)
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_unload_failed, m_controller.get_language()), name.c_str());
            return false;
        }
        #elifdef ADAM_PLATFORM_WINDOWS
        if (!FreeLibrary(reinterpret_cast<HMODULE>(handle)))
        {
            m_controller.log(log::error, get_log_event_text(log_event::module_unload_failed, m_controller.get_language()), name.c_str());
            return false;
        }
        #endif

        m_loaded_modules.erase(it);
        m_available_modules.emplace(name, std::make_pair(version, path_str));

        m_controller.log(log::info, get_log_event_text(log_event::module_unloaded, m_controller.get_language()), name.c_str());

        event evt(event_type::module_unloaded);
        auto* mod_info = evt.data_as<module::basic_info>();
        mod_info->setup(module::basic_info::available, name.c_str(), path_str.c_str(), version);
        m_controller.broadcast_event(evt);

        return true;
    }

    void registry_module_manager::clear_and_unload_all()
    {
        for (const auto& [name, mod] : m_loaded_modules)
        {
            uintptr_t handle = mod->get_module_handle();
            #ifdef ADAM_PLATFORM_LINUX
            dlclose(reinterpret_cast<void*>(handle));
            #elifdef ADAM_PLATFORM_WINDOWS
            FreeLibrary(reinterpret_cast<HMODULE>(handle));
            #endif
        }

        m_loaded_modules.clear();
        m_available_modules.clear();
        m_unavailable_modules.clear();
    }
}