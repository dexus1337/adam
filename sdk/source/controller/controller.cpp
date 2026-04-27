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
    controller::controller()
     :  m_request_command_queue(string_hashed(command_request_queue_name)),
        m_command_processing_thread(),
        m_request_command_queue_running(false),
        m_process_queues(),
        m_available_modules(),
        m_loaded_modules()
    {

    }

    controller::~controller() {}

    bool controller::start(bool async)
    {
        if (!m_request_command_queue.create(1000))
            return false;

        if (async)
        {
            m_command_processing_thread = std::thread(&controller::run_request_command_queue, this);

            return m_command_processing_thread.joinable();
        }
        else
        {
            run_request_command_queue();

            return true;
        }
    }
        
    bool controller::destroy()
    {
        m_request_command_queue_running = false;

        m_command_processing_thread.join();

        m_request_command_queue.destroy();

        return true;
    }
    
    const module* controller::get_loaded_module(const string_hashed& name) const 
    {
        auto it = m_loaded_modules.find(name);

        if (it != m_loaded_modules.end()) 
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

            if (m_loaded_modules.contains(mod->get_name()))
                continue;

            if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
                goto UNLOAD_AND_CONTINUE;

            m_available_modules.emplace(mod->get_name(), path_str);

        UNLOAD_AND_CONTINUE:
            dlclose(handle);
            continue;
            #elifdef ADAM_PLATFORM_WINDOWS
            auto handle = LoadLibraryA(std::filesystem::absolute(path).c_str());

            if (!handle)
                continue;

            // GetProcAddress uses LPCSTR (char*), even in Unicode builds
            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));

            if (!fn_get_adam_module)
                goto UNLOAD_AND_CONTINUE;

            mod = fn_get_adam_module();

            if (!mod)
                goto UNLOAD_AND_CONTINUE;

            if (m_loaded_modules.contains(mod->get_name()))
                goto UNLOAD_AND_CONTINUE;

            if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
                goto UNLOAD_AND_CONTINUE;

            m_available_modules.emplace(mod->get_name(), path_str);

        UNLOAD_AND_CONTINUE:
            FreeLibrary(handle);
            continue;
            #endif 
        }        

        return true;
    }

    bool controller::load_module(const string_hashed& name, const module** out_module)
    {
        auto it = m_available_modules.find(name);

        if (it == m_available_modules.end())
            return false;

        const auto& path_str = it->second;
        module* mod         = nullptr;

        #ifdef ADAM_PLATFORM_LINUX
        auto handle         = dlopen(path_str.c_str(), RTLD_LAZY);

        if (!handle)
            return false;

        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));

        if (!fn_get_adam_module)
            goto UNLOAD_AND_RETURN;
        
        mod = fn_get_adam_module();

        if (!mod)
            goto UNLOAD_AND_RETURN;

        if (m_loaded_modules.contains(mod->get_name()))
            goto UNLOAD_AND_RETURN;

        if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
            goto UNLOAD_AND_RETURN;

        goto SUCCESS;

    UNLOAD_AND_RETURN:
        dlclose(handle);
        #elifdef ADAM_PLATFORM_WINDOWS
        
        auto handle = LoadLibraryA(std::filesystem::absolute(path).c_str());

        if (!handle)
            return false;

        // GetProcAddress uses LPCSTR (char*), even in Unicode builds
            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));

        if (!fn_get_adam_module)
                goto UNLOAD_AND_CONTINUE;

        mod = fn_get_adam_module();

        if (!mod)
                goto UNLOAD_AND_CONTINUE;

        if (m_loaded_modules.contains(mod->get_name()))
                goto UNLOAD_AND_CONTINUE;

        if (mod->get_required_sdk_version() > ADAM_SDK_VERSION)
                goto UNLOAD_AND_CONTINUE;

        goto SUCCESS;

    UNLOAD_AND_RETURN:
        FreeLibrary(handle);
        return false;
        #endif

    SUCCESS:

        mod->m_mod_handle   = reinterpret_cast<uintptr_t>(handle);
        mod->m_str_filepath = path_str;

        m_loaded_modules.emplace(mod->get_name(), mod);

        if (out_module)
            *out_module = mod;

        return true;
    }

    void controller::run_request_command_queue()
    {
        m_request_command_queue_running = true;

        while (m_request_command_queue_running)
        {
            command_queue_request_data req_cmd;

            if (!m_request_command_queue.pop(req_cmd, 100)) // check every 100ms
                continue;
            
            auto it = m_process_queues.find(req_cmd.thread_id);
            
            // if it already is in the queue it has be running as expected
            if (it != m_process_queues.end())
                continue;

            command_queue_process_data* new_queue = new command_queue_process_data(string_hashed(command_queue_prefix + std::to_string(req_cmd.thread_id)));

            new_queue->running = true;

            if (!new_queue->queue.open())
            {
                delete new_queue;
                continue;
            }

            new_queue->command_processing_thread = std::thread(&controller::run_process_command_queue, this, new_queue);
            
            auto ins = m_process_queues.emplace(req_cmd.thread_id, new_queue);
            
            if ( !ins.second )
                continue;
            
            it = ins.first;
        }
    }

    void controller::run_process_command_queue(command_queue_process_data* data)
    {
        while (data->running)
        {
            command cmd;

            if (!data->queue.pop(cmd, 100)) // check every 100ms
                continue;

            switch (cmd.get_type())
            {
            default:
                break;
            }
        }
    }

    bool controller::stop_and_remove_process_queue(uint32_t thread_id)
    {
        auto it = m_process_queues.find(thread_id);

        if (it == m_process_queues.end())
            return false;

        it->second->running = false;

        it->second->command_processing_thread.join();

        bool result = it->second->queue.destroy();

        delete it->second;

        m_process_queues.erase(it);

        return result;
    }

}