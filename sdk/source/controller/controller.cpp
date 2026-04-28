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
#include "version/version.hpp"

namespace adam
{
    static constexpr uint32_t xtea_delta    = 0x9E3779B9;
    static constexpr uint32_t xtea_key[2]  = { 0xdeadbeef, 0x1337babe };

    /** @brief Calculates a small secret based on the XTEA-lite algorith, using ther thread id . Only this source file knows how to reverse for authorization to commands */
    uint64_t calculate_secret(os::thread_id tid)
    {
        uint32_t v0 = static_cast<uint32_t>(tid);
        uint32_t v1 = static_cast<uint32_t>(tid >> 32);
        uint32_t sum = 0;

        for (int i = 0; i < 16; i++) {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + xtea_key[sum & 1]);
            sum += xtea_delta;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + xtea_key[(sum >> 11) & 1]);
        }

        return (static_cast<uint64_t>(v1) << 32) | v0;
    }

    /** @brief Reverses the thread_id from the given secrete */
    os::thread_id reverse_secret(uint64_t secret)
    {
        uint32_t v0 = static_cast<uint32_t>(secret);
        uint32_t v1 = static_cast<uint32_t>(secret >> 32);
        uint32_t sum = xtea_delta * 16;

        for (int i = 0; i < 16; i++) {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + xtea_key[(sum >> 11) & 1]);
            sum -= xtea_delta;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + xtea_key[sum & 1]);
        }

        return static_cast<os::thread_id>((static_cast<uint64_t>(v1) << 32) | v0);
    }

    controller::controller()
     :  m_master_queue(string_hashed(master_queue_name)),
        m_master_queue_thread(),
        m_master_queue_running(false),
        m_queues_command(),
        m_queues_log(),
        m_log_outstream(std::cout.rdbuf()),
        m_available_modules(),
        m_loaded_modules()
    {

    }

    controller::~controller() {}

    bool controller::run(bool async)
    {
        if (!m_master_queue.create(1000))
            return false;

        if (async)
        {
            m_master_queue_thread = std::thread(&controller::run_master_queue, this);

            return m_master_queue_thread.joinable();
        }
        else
        {
            run_master_queue();

            return true;
        }
    }
        
    bool controller::destroy()
    {
        m_master_queue_running = false;

        m_master_queue_thread.join();

        m_master_queue.destroy();

        return true;
    }
    
    static const auto clock_offset = []() 
    {
        auto sys_now = std::chrono::system_clock::now();
        auto steady_now = std::chrono::steady_clock::now();
        return sys_now.time_since_epoch() - steady_now.time_since_epoch();
    }();

    void controller::log(const adam::log& cr_log) 
    {
        // 1. Convert steady_clock nanoseconds to system_clock
        auto tp_steady = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(cr_log.get_timestamp()));
        auto tp_sys = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(tp_steady.time_since_epoch() + clock_offset));

        // 2. Extract time for formatting
        std::time_t tt = std::chrono::system_clock::to_time_t(tp_sys);
        std::tm* local_tm = std::localtime(&tt); // Or gmtime for UTC

        // 3. Extract milliseconds for that extra "high-perf" feel
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_sys.time_since_epoch()) % 1000;

        // 4. Define log level labels and colors (ANSI codes)
        const char* level_str   = nullptr;
        const char* color_code  = nullptr;

        switch (cr_log.get_level()) 
        {
            default: return;
            case log::level::trace:     level_str = "trace";    color_code = "\033[36m";   break; // Cyan
            case log::level::info:      level_str = "info";     color_code = "\033[32m";   break; // Green
            case log::level::warning:   level_str = "warn";     color_code = "\033[33m";   break; // Yellow
            case log::level::error:     level_str = "error";    color_code = "\033[31m";   break; // Red
            case log::level::fatal:     level_str = "fatal";    color_code = "\033[1;31m"; break; // Bold Red
        }

        // 5. The "Beautified" Output
        auto str = std::format
        (   "[{:02}:{:02}:{:02}.{:03}] {}[{}]\033[0m {}\n",
            local_tm->tm_hour,
            local_tm->tm_min,
            local_tm->tm_sec,
            static_cast<int>(ms.count()),
            color_code,
            level_str,
            cr_log.get_text()
        );

        m_log_outstream << str;

        // Push the log into our sink
        for (const auto& [tid, queue] : m_queues_log_sink)
            queue->push(cr_log);
    }

    bool controller::request_queue_command_access()
    {
        master_queue mq = master_queue(string_hashed(master_queue_name));

        if (!mq.open())
            return false;

        queue_master_request_data data;
        master_queue_response resp;

        data.tid    = os::get_current_thread_id();
        data.queue  = request_command;
        data.code   = calculate_secret(data.tid);

        if (!mq.post_request(data,resp, std::chrono::milliseconds(500)))
            return false;

        mq.destroy();

        return resp == response_success;
    }

    bool controller::request_queue_log_access()
    {
        master_queue mq = master_queue(string_hashed(master_queue_name));

        if (!mq.open())
            return false;

        queue_master_request_data data;
        master_queue_response resp;

        data.tid    = os::get_current_thread_id();
        data.queue  = request_log;
        data.code   = calculate_secret(data.tid);

        if (!mq.post_request(data,resp, std::chrono::milliseconds(500)))
            return false;

        mq.destroy();

        return resp == response_success;
    }

    bool controller::request_queue_log_sink_access()
    {
        master_queue mq = master_queue(string_hashed(master_queue_name));

        if (!mq.open())
            return false;

        queue_master_request_data data;
        master_queue_response resp;

        data.tid    = os::get_current_thread_id();
        data.queue  = request_log_sink;
        data.code   = calculate_secret(data.tid);

        if (!mq.post_request(data,resp, std::chrono::milliseconds(500)))
            return false;

        mq.destroy();

        return resp == response_success;
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

            if (mod->get_required_sdk_version() > adam::sdk_version)
                goto UNLOAD_AND_CONTINUE;

            m_available_modules.emplace(mod->get_name(), std::make_pair(mod->get_version(), path_str));

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

            if (mod->get_required_sdk_version() > adam::sdk_version)
                goto UNLOAD_AND_CONTINUE;

            m_available_modules.emplace(mod->get_name(), std::make_pair(mod->get_version(), path_str));

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

        const auto& path_str = it->second.second;
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

        if (mod->get_required_sdk_version() > adam::sdk_version)
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

        if (mod->get_required_sdk_version() > adam::sdk_version)
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

    template<typename queue_type>
    bool controller::create_queue_slave
    (
        os::thread_id tid, 
        std::unordered_map<os::thread_id, queue_type*>& queue_list, 
        const char* prefix
    )
    {
        auto it = queue_list.find(tid);
        
        // if it already is in the queue it has be running as expected
        if (it != queue_list.end())
        {
            m_master_queue.response_queue().push(response_existing);

            return false;
        }

        auto* new_queue = new queue_type(string_hashed(prefix + std::to_string(tid)));

        if (!new_queue->open())
        {
            delete new_queue;

            m_master_queue.response_queue().push(response_unavailable);

            return false;
        }

        auto ins = queue_list.emplace(tid, new_queue);
        
        if ( !ins.second )
            return false;
        
        it = ins.first;

        return true;
    }

    template<typename queue_type, typename worker_fn>
    bool controller::create_queue_slave_with_worker
    (
        os::thread_id tid, 
        std::unordered_map<os::thread_id, queue_slave_instance_data<queue_type>*>& queue_list, 
        const char* prefix,
        worker_fn fn
    )
    {
        auto it = queue_list.find(tid);
        
        // if it already is in the queue it has be running as expected
        if (it != queue_list.end())
        {
            m_master_queue.response_queue().push(response_existing);

            return false;
        }

        auto* new_queue = new queue_slave_instance_data<queue_type>(string_hashed(prefix + std::to_string(tid)));

        new_queue->running = true;

        if (!new_queue->queue.open())
        {
            delete new_queue;

            m_master_queue.response_queue().push(response_unavailable);

            return false;
        }

        new_queue->queue_thread = std::thread(fn, this, new_queue);
        
        auto ins = queue_list.emplace(tid, new_queue);
        
        if ( !ins.second )
            return false;
        
        it = ins.first;

        return true;
    }

    template<typename queue_type>
    bool controller::destroy_queue_slave
    (
        os::thread_id tid, 
        std::unordered_map<os::thread_id, queue_type*>& queue_list
    )
    {
        auto it = queue_list.find(tid);

        if (it == queue_list.end())
            return false;

        bool result = it->second->destroy();

        delete it->second;

        queue_list.erase(it);

        return result;
    }

    template<typename queue_type>
    bool controller::destroy_queue_slave_with_worker
    (
        os::thread_id tid, 
        std::unordered_map<os::thread_id, queue_slave_instance_data<queue_type>*>& queue_list
    )
    {
        auto it = queue_list.find(tid);

        if (it == queue_list.end())
            return false;

        it->second->running = false;

        it->second->queue_thread.join();

        bool result = it->second->queue.destroy();

        delete it->second;

        queue_list.erase(it);

        return result;
    }

    void controller::run_master_queue()
    {
        m_master_queue_running = true;

        while (m_master_queue_running)
        {
            queue_master_request_data req;

            if (!m_master_queue.request_queue().pop(req, 100)) // check every 100ms
                continue;

            // check secret to only allow (basic) authenticated users
            if (req.tid != reverse_secret(req.code))
            {
                m_master_queue.response_queue().push(response_unauthorized);

                continue;
            }

            switch (req.queue)
            {
            case request_command:
            {
                if (!create_queue_slave_with_worker(req.tid, m_queues_command, this->queue_command_prefix, &controller::run_queue_command))
                    continue;
                
                break;
            }
            case request_log:
            {
                if (!create_queue_slave_with_worker(req.tid, m_queues_log, this->queue_log_prefix, &controller::run_queue_log))
                    continue;
                
                break;
            }
            case request_log_sink:
            {
                if (!create_queue_slave(req.tid, m_queues_log_sink, this->queue_log_sink_prefix))
                    continue;
                
                break;
            }
            default:
                m_master_queue.response_queue().push(response_unknown);
                break;
            }

            m_master_queue.response_queue().push(response_success);
        }
    }

    void controller::run_queue_command(queue_command_data* data)
    {
        while (data->running)
        {
            command cmd;

            if (!data->queue.request_queue().pop(cmd, 100)) // check every 100ms
                continue;

            response resp = response(response::invalid);

            switch (cmd.get_type())
            {
            default:
                break;
            }

            data->queue.response_queue().push(resp);
        }
    }

    void controller::run_queue_log(queue_log_data* data)
    {
        while (data->running)
        {
            adam::log clog;

            if (!data->queue.pop(clog, 100)) // check every 100ms
                continue;

            this->log(clog);
        }
    }
}

