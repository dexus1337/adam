#include "controller/controller.hpp"


#include <filesystem>
#include <vector>
#include <iostream>

#include "controller/controller-module-manager.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "version/version.hpp"
#include "resources/language-strings.hpp"


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
        m_modules(*this),
        m_registry(*this),
        m_dispatcher()
    {
        m_dispatcher.register_default_handlers();
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
        command_context ctx { data->tid, m_registry, m_lang, *this, {} };

        std::vector<command> cmds;

        static constexpr int command_buffer_size = 256; // should be enough for most commands and their extensions, if not it will just resize automatically, no big deal

        cmds.reserve(command_buffer_size);
        
        for (size_t i = 0; i < command_buffer_size; i++)
            cmds.emplace_back(); // default construct the commands to ensure the data buffer is allocated, so we can directly write into it when popping from the queue
        
        while (data->queue.is_active())
        {
            size_t cmd_idx = 0;

            if (!data->queue.request_queue().pop(cmds[cmd_idx], 100)) // check every 100ms
                continue;

            while (cmds[cmd_idx].is_extended())
            {
                cmd_idx++;
                if (cmd_idx >= cmds.size())
                {
                    cmds.emplace_back();
                }
                
                if (!data->queue.request_queue().pop(cmds[cmd_idx], 100))
                {
                    cmd_idx--; // Backtrack the index since this fetch failed
                    break;
                }
            }

            response resp = m_dispatcher.dispatch(cmds.data(), cmd_idx + 1, ctx);

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
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message("controller::log_event", val, lang);
    }
}
