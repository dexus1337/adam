#include "controller/controller-module-manager.hpp"
#include "controller/controller.hpp"
#include "version/version.hpp"
#include "resources/language-strings.hpp"

#include <array>
#include <filesystem>
#include <vector>
#include <format>

#ifdef   ADAM_PLATFORM_LINUX
#include <dlfcn.h>
#elifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam
{
    controller_module_manager::controller_module_manager(controller& ctrl) : m_controller(ctrl) {}

    controller_module_manager::~controller_module_manager() {}

    std::string_view controller_module_manager::get_log_event_text(log_event event, language lang)
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
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message("controller_module_manager::log_event", val, lang);
    }

    const module* controller_module_manager::get_loaded_module(const string_hashed& name) const 
    {
        auto it = m_loaded_modules.find(name);

        if (it != m_loaded_modules.end()) 
            return it->second;

        return nullptr;
    }

    bool controller_module_manager::scan_for_modules(string_hashed::view directory) 
    {
        std::vector<std::filesystem::path> possible_module_paths;

        try 
        {
            if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) 
            {
                for (const auto& entry : std::filesystem::directory_iterator(directory)) 
                {
                    if (!entry.is_regular_file()) continue;

                    #ifdef ADAM_PLATFORM_WINDOWS
                    if (entry.path().extension() != ".dll") continue;
                    #else
                    if (entry.path().extension() != ".so") continue;
                    #endif

                    possible_module_paths.push_back(entry.path());
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) { return false; }

        for (const auto& path : possible_module_paths)
        {
            module* mod         = nullptr;
            auto path_str       = std::string(path.string());

            #ifdef ADAM_PLATFORM_LINUX
            auto handle = dlopen(path_str.c_str(), RTLD_LAZY);
            if (!handle) return false;
            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));
            #elifdef ADAM_PLATFORM_WINDOWS
            auto handle = LoadLibraryW(std::filesystem::absolute(path).c_str());
            if (!handle) return false;
            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));
            #endif

            mod = fn_get_adam_module();
            if (!mod) goto UNLOAD_AND_CONTINUE;
            if (m_loaded_modules.contains(mod->get_name())) continue;

            if (mod->get_required_sdk_version() > adam::sdk_version)
            {
                auto req_maj = adam::get_major(mod->get_required_sdk_version());
                auto req_min = adam::get_minor(mod->get_required_sdk_version());
                auto req_pat = adam::get_patch(mod->get_required_sdk_version());
                auto sdk_maj = adam::get_major(adam::sdk_version);
                auto sdk_min = adam::get_minor(adam::sdk_version);
                auto sdk_pat = adam::get_patch(adam::sdk_version);

                m_controller.log(log::warning, std::vformat(get_log_event_text(log_event::module_requires_newer_sdk, m_controller.m_lang), std::make_format_args(
                    path_str, req_maj, req_min, req_pat, sdk_maj, sdk_min, sdk_pat)));

                m_unavailable_modules.emplace(mod->get_name(), std::make_pair(1, path_str));
                goto UNLOAD_AND_CONTINUE;
            }

            m_available_modules.emplace(mod->get_name(), std::make_pair(mod->get_version(), path_str));
            continue;

        UNLOAD_AND_CONTINUE:
            #ifdef ADAM_PLATFORM_LINUX
            dlclose(handle);
            #elifdef ADAM_PLATFORM_WINDOWS
            FreeLibrary(handle);
            #endif
            continue;
        }        
        return true;
    }

    bool controller_module_manager::load_module(const string_hashed& name, const module** out_module)
    {
        auto it = m_available_modules.find(name);
        if (it == m_available_modules.end()) return false;

        const auto& path_str = it->second.second;
        module* mod          = nullptr;

        #ifdef ADAM_PLATFORM_LINUX
        auto handle = dlopen(path_str.c_str(), RTLD_LAZY);
        if (!handle) return false;
        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));
        #elifdef ADAM_PLATFORM_WINDOWS
        auto handle = LoadLibraryA(path_str.c_str());
        if (!handle) return false;
        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));
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

            m_controller.log(log::error, std::vformat(get_log_event_text(log_event::module_requires_newer_sdk_cannot_load, m_controller.m_lang), std::make_format_args(
                path_str, req_maj, req_min, req_pat, sdk_maj, sdk_min, sdk_pat)));
            return false;
        }

        mod->m_mod_handle   = reinterpret_cast<uintptr_t>(handle);
        mod->m_str_filepath = path_str;
        m_loaded_modules.emplace(mod->get_name(), mod);
        if (out_module) *out_module = mod;
        return true;

    UNLOAD_AND_RETURN:
        #ifdef ADAM_PLATFORM_LINUX
        dlclose(handle);
        #elifdef ADAM_PLATFORM_WINDOWS
        FreeLibrary(handle);
        #endif
        return false;
    }
}