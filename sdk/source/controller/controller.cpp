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
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/inspector.hpp"


namespace adam
{
    static constexpr uint16_t xtea_delta    = 0x9E37;
    static constexpr uint16_t xtea_key[2]   = { 0xbeef, 0xbabe };

    /** @brief Calculates a small secret based on the XTEA-lite algorith, using ther thread id . Only this source file knows how to reverse for authorization to commands */
    uint32_t calculate_secret(os::thread_id tid)
    {
        uint16_t v0 = static_cast<uint16_t>(tid);
        uint16_t v1 = static_cast<uint16_t>(tid >> 16);
        uint16_t sum = 0;

        for (int i = 0; i < 16; i++) 
        {
            v0 += static_cast<uint16_t>((((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + xtea_key[sum & 1]));
            sum += xtea_delta;
            v1 += static_cast<uint16_t>((((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + xtea_key[(sum >> 11) & 1]));
        }

        return (static_cast<uint32_t>(v1) << 16) | v0;
    }

    /** @brief Reverses the thread_id from the given secrete */
    os::thread_id reverse_secret(uint32_t secret)
    {
        uint16_t v0 = static_cast<uint16_t>(secret);
        uint16_t v1 = static_cast<uint16_t>(secret >> 16);
        uint16_t sum = static_cast<uint16_t>(xtea_delta * 16);

        for (int i = 0; i < 16; i++) 
        {
            v1 -= static_cast<uint16_t>((((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + xtea_key[(sum >> 11) & 1]));
            sum -= xtea_delta;
            v0 -= static_cast<uint16_t>((((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + xtea_key[sum & 1]));
        }

        return static_cast<os::thread_id>((static_cast<uint32_t>(v1) << 16) | v0);
    }

    controller& controller::get()
    {
        static controller instance;
        return instance;
    }

    controller::controller()
     :  m_master_queue(string_hashed(master_queue_name)),
        m_master_queue_thread(),
        m_queues_command(),
        m_queues_log(),
        m_log_outstream(std::cout.rdbuf()),
        m_lang(language_german),
        m_available_modules(),
        m_loaded_modules(),
        m_registry(this)
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
        m_master_queue.disable();

        if (m_master_queue_thread.joinable())
            m_master_queue_thread.join();

        m_master_queue.destroy();

        return true;
    }
    
    void controller::log(const adam::log& cr_log) 
    {
        stream_log(cr_log, m_log_outstream);

        // Push the log into our sink
        for (const auto& [tid, queue] : m_queues_log_sink)
        {
            auto wanted_lvl = queue->get_metadata()->load(std::memory_order_relaxed);

            if (cr_log.get_level() < wanted_lvl)
                continue;
            
            queue->push(cr_log);
        }
    }

    controller::status controller::request_master_queue(master_queue_request mqr)
    {
        status resp     = status_queue_unavailable;
        master_queue mq = master_queue(string_hashed(master_queue_name));

        if (!mq.open())
        {
            int mqr_val = static_cast<int>(mqr);
            adam::stream_log(log::trace, std::vformat(get_log_event_text(log_event::master_queue_open_failed, controller::get().m_lang), std::make_format_args(mqr_val)), std::cout);
            return resp;
        }

        queue_master_request_data data;

        data.tid    = os::get_current_thread_id();
        data.queue  = mqr;
        data.code   = calculate_secret(data.tid);

        auto pres = mq.post_request(data, resp, 1000);

        if (!pres)
        {
            int mqr_val  = static_cast<int>(mqr);
            int resp_val = static_cast<int>(resp);
            adam::stream_log(log::trace, std::vformat(get_log_event_text(log_event::master_queue_request_failed, controller::get().m_lang), std::make_format_args(mqr_val, resp_val)), std::cout);
        }
        
        mq.destroy();

        return resp;
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

    bool controller::scan_for_modules(string_hashed::view directory) 
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
            auto path_str       = std::string(path.string());

            #ifdef ADAM_PLATFORM_LINUX
            auto handle = dlopen(path_str.c_str(), RTLD_LAZY);

            if (!handle)
                return false;

            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));
            #elifdef ADAM_PLATFORM_WINDOWS

            auto handle = LoadLibraryW(std::filesystem::absolute(path).c_str());

            if (!handle)
                return false;

            auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));
            #endif

            mod = fn_get_adam_module();

            if (!mod)
                goto UNLOAD_AND_CONTINUE;

            if (m_loaded_modules.contains(mod->get_name()))
                continue;

            if (mod->get_required_sdk_version() > adam::sdk_version)
            {
                auto req_maj = adam::get_major(mod->get_required_sdk_version());
                auto req_min = adam::get_minor(mod->get_required_sdk_version());
                auto req_pat = adam::get_patch(mod->get_required_sdk_version());
                auto sdk_maj = adam::get_major(adam::sdk_version);
                auto sdk_min = adam::get_minor(adam::sdk_version);
                auto sdk_pat = adam::get_patch(adam::sdk_version);

                this->log(log::warning, std::vformat(get_log_event_text(log_event::module_requires_newer_sdk, m_lang), std::make_format_args(
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

    bool controller::load_module(const string_hashed& name, const module** out_module)
    {
        auto it = m_available_modules.find(name);

        if (it == m_available_modules.end())
            return false;

        const auto& path_str = it->second.second;
        module* mod          = nullptr;

        #ifdef ADAM_PLATFORM_LINUX
        auto handle = dlopen(path_str.c_str(), RTLD_LAZY);

        if (!handle)
            return false;

        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(dlsym(handle, module::entry_point_name));
        #elifdef ADAM_PLATFORM_WINDOWS

        auto handle = LoadLibraryA(path_str.c_str());

        if (!handle)
            return false;

        // GetProcAddress uses LPCSTR (char*), even in Unicode builds
        auto fn_get_adam_module = reinterpret_cast<module::get_adam_module_fn>(GetProcAddress(handle, module::entry_point_name));
        #endif

        if (!fn_get_adam_module)
            goto UNLOAD_AND_RETURN;
        
        mod = fn_get_adam_module();

        if (!mod)
            goto UNLOAD_AND_RETURN;

        if (m_loaded_modules.contains(mod->get_name()))
            goto UNLOAD_AND_RETURN;

        if (mod->get_required_sdk_version() > adam::sdk_version)
        {
            auto req_maj = adam::get_major(mod->get_required_sdk_version());
            auto req_min = adam::get_minor(mod->get_required_sdk_version());
            auto req_pat = adam::get_patch(mod->get_required_sdk_version());
            auto sdk_maj = adam::get_major(adam::sdk_version);
            auto sdk_min = adam::get_minor(adam::sdk_version);
            auto sdk_pat = adam::get_patch(adam::sdk_version);

            this->log(log::error, std::vformat(get_log_event_text(log_event::module_requires_newer_sdk_cannot_load, m_lang), std::make_format_args(
                path_str, req_maj, req_min, req_pat, sdk_maj, sdk_min, sdk_pat)));

            return false;
        }

        goto SUCCESS;

    UNLOAD_AND_RETURN:
        #ifdef ADAM_PLATFORM_LINUX
        dlclose(handle);
        #elifdef ADAM_PLATFORM_WINDOWS
        FreeLibrary(handle);
        #endif

        return false;

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
            m_master_queue.response_queue().push(status_queue_existing);

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_already_exists, m_lang), std::make_format_args(tid))));

            return false;
        }

        auto* new_queue = new queue_type(string_hashed(prefix + std::to_string(tid)));

        if (!new_queue->open())
        {
            delete new_queue;

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_failed_to_open, m_lang), std::make_format_args(tid))));

            m_master_queue.response_queue().push(status_queue_unavailable);

            return false;
        }

        auto ins = queue_list.emplace(tid, new_queue);
        
        if (!ins.second)
        {
            delete new_queue;

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_failed_to_insert, m_lang), std::make_format_args(tid))));

            m_master_queue.response_queue().push(status_queue_failed_create);

            return false;
        }

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
            m_master_queue.response_queue().push(status_queue_existing);

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_worker_already_exists, m_lang), std::make_format_args(tid))));

            return false;
        }

        auto* new_queue = new queue_slave_instance_data<queue_type>(string_hashed(prefix + std::to_string(tid)), tid);

        if (!new_queue->queue.open())
        {
            delete new_queue;

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_worker_failed_to_open, m_lang), std::make_format_args(tid))));

            m_master_queue.response_queue().push(status_queue_unavailable);

            return false;
        }

        new_queue->queue_thread = std::thread(fn, this, new_queue);
        
        auto ins = queue_list.emplace(tid, new_queue);

        if (!ins.second)
        {
            new_queue->queue.disable();

            new_queue->queue_thread.join();

            delete new_queue;

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_worker_failed_to_insert, m_lang), std::make_format_args(tid))));

            m_master_queue.response_queue().push(status_queue_failed_create);

            return false;
        }

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
        {
            m_master_queue.response_queue().push(status_queue_not_existing);

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_does_not_exist, m_lang), std::make_format_args(tid))));

            return false;
        }

        it->second->disable();

        if (!it->second->destroy())
        {
            m_master_queue.response_queue().push(status_queue_failed_destroy);

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_failed_to_destroy, m_lang), std::make_format_args(tid))));

            return false;
        }

        delete it->second;

        queue_list.erase(it);

        return true;
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
        {
            m_master_queue.response_queue().push(status_queue_not_existing);

            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_worker_does_not_exist, m_lang), std::make_format_args(tid))));

            return false;
        }

        it->second->queue.disable();

        it->second->queue_thread.join();

        if (!it->second->queue.destroy())
            debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_worker_failed_to_destroy, m_lang), std::make_format_args(tid))));

        delete it->second;

        queue_list.erase(it);

        return true;
    }

    void controller::run_master_queue()
    {
        while (m_master_queue.is_active())
        {
            queue_master_request_data req;

            if (!m_master_queue.request_queue().pop(req, 100)) // check every 100ms
                continue;

            // check secret to only allow (basic) authenticated users
            if (req.tid != reverse_secret(req.code))
            {
                m_master_queue.response_queue().push(status_queue_unauthorized);

                debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::thread_auth_failed, m_lang), std::make_format_args(req.tid))));

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
            case request_command_destroy:
            {
                if (!destroy_queue_slave_with_worker(req.tid, m_queues_command))
                    continue;
                
                break;
            }
            case request_log_destroy:
            {
                if (!destroy_queue_slave_with_worker(req.tid, m_queues_log))
                    continue;
                
                break;
            }
            case request_log_sink_destroy:
            {
                if (!destroy_queue_slave(req.tid, m_queues_log_sink))
                    continue;
                
                break;
            }
            default:
                m_master_queue.response_queue().push(status_unknown_master_request);
                break;
            }

            if (req.queue % 2)
            {
                int queue_val = static_cast<int>(req.queue);
                debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_created, m_lang), std::make_format_args(req.tid, queue_val))));
            }
            else
            {
                int queue_val = static_cast<int>(req.queue) - 1;
                debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::slave_queue_destroyed, m_lang), std::make_format_args(req.tid, queue_val))));
            }

            m_master_queue.response_queue().push(status_success);
        }
    }

    void controller::run_queue_command(queue_command_data* data)
    {
        thread_local std::unordered_map<string_hashed::hash_datatype, std::shared_ptr<data_inspector>> thread_inspectors;

        while (data->queue.is_active())
        {
            command cmd;

            if (!data->queue.request_queue().pop(cmd, 100)) // check every 100ms
                continue;

            response resp = response(response::invalid);

            switch (cmd.get_type())
            {
                case command::set_language:
                {
                    m_lang  = *cmd.get_data_as<language>();
                    resp    = response::success;

                    this->log(log::info, get_log_event_text(log_event::language_changed, m_lang));

                    break;
                }
                case command::inspector_create:
                {
                    auto params = cmd.get_data_as<command::inspector_create_data>();

                    // First check if the port exists
                    auto port = m_registry.ports().find(params->port);

                    if (port == m_registry.ports().end())
                    {
                        resp = response::unknown;

                        uint64_t port_hash = static_cast<uint64_t>(params->port);
                        debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_create_failed_port_unknown, m_lang), std::make_format_args(data->tid, port_hash))));

                        break;
                    }

                    const auto& port_name = port->second->get_name();
                    auto new_inspector = std::make_shared<data_inspector>();

                    // Try to open the inspector with the given port, if it fails respond with failed
                    if (!new_inspector->open(port_name))
                    {
                        resp = response::failed;

                        debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_create_failed_open, m_lang), std::make_format_args(data->tid, port_name))));

                        break;
                    }

                    // At this point the inspector seems to be valid, so data can be forwareded to it
                    port->second->inspectors().push_back(new_inspector);

                    thread_inspectors.emplace(params->port, new_inspector);

                    resp = response::success;

                    debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_created, m_lang), std::make_format_args(data->tid, port_name))));

                    break;
                }
                case command::inspector_destroy:
                {
                    auto params = cmd.get_data_as<command::inspector_destroy_data>();

                    // First check if the port exists
                    auto port = m_registry.ports().find(params->port);
                    if (port == m_registry.ports().end())
                    {
                        resp = response::unknown;
                        
                        uint64_t port_hash = static_cast<uint64_t>(params->port);
                        debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_destroy_failed_port_unknown, m_lang), std::make_format_args(data->tid, port_hash))));

                        break;
                    }

                    const auto& port_name = port->second->get_name();

                    // Locate the inspector managed by this thread
                    auto it = thread_inspectors.find(params->port);
                    if (it == thread_inspectors.end())
                    {
                        resp = response::failed;
                        
                        debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_destroy_failed_not_found, m_lang), std::make_format_args(data->tid, port_name))));

                        break;
                    }

                    // Safely remove the raw pointer from the port's routing list
                    port->second->inspectors().remove(it->second);

                    // Erase the unique_ptr from the map. Once the port releases its shared_ptr, the inspector will be destroyed and frees the shared memory too.
                    thread_inspectors.erase(it);

                    resp = response::success;
                    
                    debug_statement(this->log(log::trace, std::vformat(get_log_event_text(log_event::inspector_destroyed, m_lang), std::make_format_args(data->tid, port_name))));

                    break;
                }
                default:
                    break;
            }

            data->queue.response_queue().push(resp);
        }
    }

    void controller::run_queue_log(queue_log_data* data)
    {
        while (data->queue.is_active())
        {
            adam::log clog;

            if (!data->queue.pop(clog, 100)) // check every 100ms
                continue;

            this->log(clog);
        }
    }
    
    std::string_view controller::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            { 
                static_cast<int>(log_event::language_changed),
                { "Language changed to: english!",                  "Sprache geändert zu: Deutsch." }
            },
            { 
                static_cast<int>(log_event::thread_auth_failed),
                { "Thread {:d} did not authenticate correctly.",    "Thread {:d} hat sich nicht korrekt authentifiziert." }
            },
            { 
                static_cast<int>(log_event::slave_queue_created),
                { "Thread {:d} did successfully create queue of type {:d}.",    "Thread {:d} hat eine Warteschlange vom Typ {:d} erfolgreich erstellt." }
            },
            { 
                static_cast<int>(log_event::slave_queue_destroyed),
                { "Thread {:d} did successfully destroy queue of type {:d}.",   "Thread {:d} hat eine Warteschlange vom Typ {:d} erfolgreich entfernt." }
            },
            {
                static_cast<int>(log_event::master_queue_open_failed),
                { "Cannot open the master queue for access to queue {:d}.",     "Die Haupt-Warteschlange für den Zugriff auf Warteschlange {:d} konnte nicht geöffnet werden." }
            },
            {
                static_cast<int>(log_event::master_queue_request_failed),
                { "Request for queue {:d} failed, response {:d}.",              "Anfrage für Warteschlange {:d} fehlgeschlagen, Antwort {:d}." }
            },
            {
                static_cast<int>(log_event::module_requires_newer_sdk),
                { "Module \"{}\" requires SDK {:d}.{:d}.{:d}, this is {:d}.{:d}.{:d}", "Modul \"{}\" benötigt SDK {:d}.{:d}.{:d}, dies ist {:d}.{:d}.{:d}" }
            },
            {
                static_cast<int>(log_event::module_requires_newer_sdk_cannot_load),
                { "Module \"{}\" requires SDK {:d}.{:d}.{:d}, this is {:d}.{:d}.{:d}. Cannot be loaded!", "Modul \"{}\" benötigt SDK {:d}.{:d}.{:d}, dies ist {:d}.{:d}.{:d}. Kann nicht geladen werden!" }
            },
            {
                static_cast<int>(log_event::slave_queue_already_exists),
                { "Client {:d} tried to create a queue that already exists.",   "Client {:d} hat versucht, eine Warteschlange zu erstellen, die bereits existiert." }
            },
            {
                static_cast<int>(log_event::slave_queue_failed_to_open),
                { "Queue for client {:d} failed to open.",                      "Warteschlange für Client {:d} konnte nicht geöffnet werden." }
            },
            {
                static_cast<int>(log_event::slave_queue_failed_to_insert),
                { "Queue for client {:d} failed to insert to database.",        "Warteschlange für Client {:d} konnte nicht in die Datenbank eingefügt werden." }
            },
            {
                static_cast<int>(log_event::slave_queue_worker_already_exists),
                { "Client {:d} tried to create a queue + worker that already exists.", "Client {:d} hat versucht, eine Warteschlange + Worker zu erstellen, die bereits existieren." }
            },
            {
                static_cast<int>(log_event::slave_queue_worker_failed_to_open),
                { "Queue + worker for client {:d} failed to open.",             "Warteschlange + Worker für Client {:d} konnten nicht geöffnet werden." }
            },
            {
                static_cast<int>(log_event::slave_queue_worker_failed_to_insert),
                { "Queue + worker for client {:d} failed to insert to database.", "Warteschlange + Worker für Client {:d} konnten nicht in die Datenbank eingefügt werden." }
            },
            {
                static_cast<int>(log_event::slave_queue_does_not_exist),
                { "Client {:d} tried to destroy a queue that doesnt exists.",   "Client {:d} hat versucht, eine Warteschlange zu entfernen, die nicht existiert." }
            },
            {
                static_cast<int>(log_event::slave_queue_failed_to_destroy),
                { "Failed to destroy queue of client {:d}.",                    "Fehler beim Entfernen der Warteschlange von Client {:d}." }
            },
            {
                static_cast<int>(log_event::slave_queue_worker_does_not_exist),
                { "Client {:d} tried to destroy a queue + worker that doesnt exists.", "Client {:d} hat versucht, eine Warteschlange + Worker zu entfernen, die nicht existieren." }
            },
            {
                static_cast<int>(log_event::slave_queue_worker_failed_to_destroy),
                { "Failed to destroy queue + worker of client {:d}. Ignoring.", "Fehler beim Entfernen der Warteschlange + Worker von Client {:d}. Wird ignoriert." }
            },
            {
                static_cast<int>(log_event::inspector_created),
                { "Thread {:d} successfully created inspector for port \"{}\".", "Thread {:d} hat erfolgreich einen Inspektor für Port \"{}\" erstellt." }
            },
            {
                static_cast<int>(log_event::inspector_destroyed),
                { "Thread {:d} successfully destroyed inspector for port \"{}\".", "Thread {:d} hat erfolgreich den Inspektor für Port \"{}\" entfernt." }
            },
            {
                static_cast<int>(log_event::inspector_create_failed_port_unknown),
                { "Thread {:d} failed to create inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht erstellen: Unbekannter Port-Hash {:d}." }
            },
            {
                static_cast<int>(log_event::inspector_create_failed_open),
                { "Thread {:d} failed to create inspector: Could not open queue for port \"{}\".", "Thread {:d} konnte Inspektor nicht erstellen: Warteschlange für Port \"{}\" konnte nicht geöffnet werden." }
            },
            {
                static_cast<int>(log_event::inspector_destroy_failed_port_unknown),
                { "Thread {:d} failed to destroy inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht entfernen: Unbekannter Port-Hash {:d}." }
            },
            {
                static_cast<int>(log_event::inspector_destroy_failed_not_found),
                { "Thread {:d} failed to destroy inspector: Inspector not found for port \"{}\".", "Thread {:d} konnte Inspektor nicht entfernen: Kein Inspektor für Port \"{}\" gefunden." }
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message(_TYPEINFO "controller::log_event", val, lang);
    }
}
