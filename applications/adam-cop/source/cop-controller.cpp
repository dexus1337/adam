/**
 * @file    cop-controller.cpp
 * @author  dexus1337
 * @brief   Application state controller implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "cop-controller.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    static const configuration_parameter_list& get_default_parameters()
    {
        static configuration_parameter_list params = []()
        {
            configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_boolean>("show_performance"_ct, false));
            p.add(std::make_unique<configuration_parameter_boolean>("show_control_panel"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_waypoints"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_sites"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("auto_detect_sites"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_cache_stats"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_jump_coords"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_asterix"_ct, true));
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_location"_ct, 1));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_x"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_y"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_content"_ct, 7));
            p.add(std::make_unique<configuration_parameter_integer>("fps_limit"_ct, 2)); // 60 FPS default
            p.add(std::make_unique<configuration_parameter_integer>("map_projection"_ct, 1)); // 0 = Equirectangular, 1 = Mercator
            p.add(std::make_unique<configuration_parameter_boolean>("show_grid"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_coastlines"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_land_fill"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_scale_bar"_ct, true));
            p.add(std::make_unique<configuration_parameter_integer>("language"_ct, static_cast<int>(adam::language_english)));
            p.add(std::make_unique<configuration_parameter_string>("theme"_ct, "dark"_ct));
            p.add(std::make_unique<configuration_parameter_integer>("gui_mode"_ct, 0)); // 0 = Default, 1 = Immediate
            p.add(std::make_unique<configuration_parameter_double>("font_scale"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_integer>("window_x"_ct, -1));
            p.add(std::make_unique<configuration_parameter_integer>("window_y"_ct, -1));
            p.add(std::make_unique<configuration_parameter_integer>("window_w"_ct, 1280));
            p.add(std::make_unique<configuration_parameter_integer>("window_h"_ct, 720));
            p.add(std::make_unique<configuration_parameter_boolean>("window_maximized"_ct, false));
            p.add(std::make_unique<configuration_parameter_string>("docking_layout"_ct, ""_ct));
            p.add(std::make_unique<configuration_parameter_double>("map_lat"_ct, 20.0));
            p.add(std::make_unique<configuration_parameter_double>("map_lon"_ct, 10.0));
            p.add(std::make_unique<configuration_parameter_double>("map_zoom"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_list_sorted>("waypoints"_ct));
            p.add(std::make_unique<configuration_parameter_list_sorted>("sites"_ct));
            
            p.add(std::make_unique<configuration_parameter_integer>("map_layer_0_provider"_ct, 0));
            p.add(std::make_unique<configuration_parameter_double>("map_layer_0_opacity"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_boolean>("map_layer_0_visible"_ct, true));

            p.add(std::make_unique<configuration_parameter_integer>("map_layer_1_provider"_ct, 1));
            p.add(std::make_unique<configuration_parameter_double>("map_layer_1_opacity"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_boolean>("map_layer_1_visible"_ct, false));

            p.add(std::make_unique<configuration_parameter_integer>("map_layer_2_provider"_ct, 2));
            p.add(std::make_unique<configuration_parameter_double>("map_layer_2_opacity"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_boolean>("map_layer_2_visible"_ct, false));

            p.add(std::make_unique<configuration_parameter_integer>("map_layer_3_provider"_ct, 3));
            p.add(std::make_unique<configuration_parameter_double>("map_layer_3_opacity"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_boolean>("map_layer_3_visible"_ct, false));

            return p;
        }();
        return params;
    }

    cop_controller::cop_controller()
        : configuration_item("adam_cop_controller", get_default_parameters())
        , m_commander("COP"_ct)
        , m_running(false)
        , m_commander_active(false)
        , m_p_auto_detect_sites(get_parameter<configuration_parameter_boolean>("auto_detect_sites"_ct))
        , m_p_waypoints_list(get_parameter<configuration_parameter_list_sorted>("waypoints"_ct))
        , m_p_sites_list(get_parameter<configuration_parameter_list_sorted>("sites"_ct))
    {
        load("adam-cop-config.adamcopcfg");
        load_waypoints_from_config();
        load_sites_from_config();
    }

    cop_controller::~cop_controller()
    {
        sync_waypoints_to_config();
        sync_sites_to_config();
        save("adam-cop-config.adamcopcfg");
        stop();
    }

    void cop_controller::start()
    {
        if (m_running.exchange(true))
        {
            return;
        }

        m_worker_thread = std::thread(&cop_controller::update_loop, this);
    }

    void cop_controller::stop()
    {
        if (!m_running.exchange(false))
        {
            return;
        }

        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }
    }

    bool cop_controller::is_commander_active() const
    {
        return m_commander_active.load();
    }

    std::vector<log_entry> cop_controller::get_log_history() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_log_history;
    }

    bool cop_controller::is_log_history_empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_log_history.empty();
    }

    void cop_controller::clear_log_history()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_log_history.clear();
    }

    void cop_controller::request_redraw()
    {
        if (!m_redraw_callback)
        {
            return;
        }

        m_redraw_callback();
    }

    void cop_controller::set_redraw_callback(std::function<void()> cb)
    {
        m_redraw_callback = std::move(cb);
    }

    void cop_controller::enqueue_commander_action(std::function<void()> action)
    {
        if (!action)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_deferred_commander_actions.push_back(std::move(action));
    }

    void cop_controller::add_waypoint(std::unique_ptr<waypoint> wp)
    {
        m_waypoints.push_back(std::move(wp));
        save_config();
    }

    void cop_controller::remove_waypoint(adam::string_hash item_hash)
    {
        auto it = std::find_if(m_waypoints.begin(), m_waypoints.end(), [item_hash](const std::unique_ptr<waypoint>& wp) 
        { 
            return wp->get_name().get_hash() == item_hash; 
        });

        if (it != m_waypoints.end())
        {
            m_waypoints.erase(it);
            save_config();
        }
    }

    void cop_controller::clear_waypoints()
    {
        m_waypoints.clear();
        sync_waypoints_to_config();
        save_config();
    }

    void cop_controller::save_config()
    {
        sync_waypoints_to_config();
        sync_sites_to_config();
        save("adam-cop-config.adamcopcfg");
    }

    void cop_controller::load_waypoints_from_config()
    {
        m_waypoints.clear();
        if (!m_p_waypoints_list)
        {
            return;
        }

        for (const auto& hash : m_p_waypoints_list->get_order())
        {
            auto* p = m_p_waypoints_list->get<configuration_parameter_list>(hash);
            if (!p)
            {
                continue;
            }

            auto wp = std::make_unique<waypoint>(p->get_name());
            wp->parameters().copy_values_from(p);
            m_waypoints.push_back(std::move(wp));
        }
    }

    void cop_controller::sync_waypoints_to_config()
    {
        if (!m_p_waypoints_list)
        {
            return;
        }

        m_p_waypoints_list->clear();
        uint32_t wp_id = 0;
        for (const auto& wp : m_waypoints)
        {
            if (!wp)
            {
                continue;
            }

            auto p = wp->get_parameters().clone();
            
            char id_str[128];
            snprintf(id_str, sizeof(id_str), "wp_%u_%f_%f", ++wp_id, wp->get_lat(), wp->get_lon());
            p->set_name(adam::string_hashed(&id_str[0]));
            
            m_p_waypoints_list->add(std::move(p));
        }
    }

    void cop_controller::add_site(std::unique_ptr<site> s)
    {
        if (!s)
        {
            return;
        }

        m_sites.push_back(std::move(s));
        save_config();
        request_redraw();
    }

    void cop_controller::remove_site(adam::string_hash item_hash)
    {
        auto it = std::find_if(m_sites.begin(), m_sites.end(), [item_hash](const std::unique_ptr<site>& s) 
        { 
            return s->get_name().get_hash() == item_hash; 
        });

        if (it != m_sites.end())
        {
            m_sites.erase(it);
            save_config();
            request_redraw();
        }
    }

    void cop_controller::clear_sites()
    {
        m_sites.clear();
        sync_sites_to_config();
        save_config();
        request_redraw();
    }

    bool cop_controller::is_auto_detect_sites() const
    {
        if (!m_p_auto_detect_sites)
        {
            return true;
        }

        return m_p_auto_detect_sites->get_value();
    }

    void cop_controller::set_auto_detect_sites(bool enable)
    {
        if (m_p_auto_detect_sites)
        {
            m_p_auto_detect_sites->set_value(enable);
        }

        save_config();
        request_redraw();
    }

    void cop_controller::auto_detect_sites_from_streams()
    {
        enable_all_radar_streams(true);
        set_auto_detect_sites(true);
    }

    void cop_controller::load_sites_from_config()
    {
        m_sites.clear();
        if (!m_p_sites_list)
        {
            return;
        }

        for (const auto& hash : m_p_sites_list->get_order())
        {
            auto* p = m_p_sites_list->get<configuration_parameter_list>(hash);
            if (!p)
            {
                continue;
            }

            auto s = std::make_unique<site>(p->get_name());
            s->parameters().copy_values_from(p);
            m_sites.push_back(std::move(s));
        }
    }

    void cop_controller::sync_sites_to_config()
    {
        if (!m_p_sites_list)
        {
            return;
        }

        m_p_sites_list->clear();
        uint32_t site_id = 0;
        for (const auto& s : m_sites)
        {
            if (!s)
            {
                continue;
            }

            auto p = s->get_parameters().clone();
            
            char id_str[128];
            snprintf(id_str, sizeof(id_str), "site_%u_%llu_%f_%f", ++site_id, static_cast<unsigned long long>(s->get_sacsic()), s->get_lat(), s->get_lon());
            p->set_name(adam::string_hashed(&id_str[0]));
            
            m_p_sites_list->add(std::move(p));
        }
    }

    void cop_controller::update_loop()
    {
        auto last_reconnect_attempt = std::chrono::steady_clock::now() - std::chrono::seconds(5);

        while (m_running.load(std::memory_order_relaxed))
        {
            auto now = std::chrono::steady_clock::now();

            bool commander_active = m_commander.is_active();
            bool log_sink_active = m_log_sink.is_active();

            if (!commander_active || !log_sink_active)
            {
                if (std::chrono::duration_cast<std::chrono::seconds>(now - last_reconnect_attempt).count() >= 5)
                {
                    last_reconnect_attempt = now;

                    if (!commander_active)
                    {
                        if (log_sink_active)
                        {
                            m_log_sink.destroy();
                            log_sink_active = false;
                        }

                        {
                            std::lock_guard<std::mutex> lock(m_radar_mutex);
                            m_radar_inspectors.clear();
                        }

                        m_commander.destroy();
                        m_commander.connect();
                        commander_active = m_commander.is_active();
                    }

                    if (!log_sink_active)
                    {
                        m_log_sink.connect();
                        log_sink_active = m_log_sink.is_active();

                        if (log_sink_active && m_log_sink.queue().metadata())
                        {
                            adam::log discard;
                            while (m_log_sink.queue().pop(discard, 0)) {}
                        }
                    }
                }
            }

            if (commander_active != m_commander_active.load(std::memory_order_relaxed))
            {
                request_redraw();
            }

            if (commander_active)
            {
                std::vector<std::function<void()>> actions_to_run;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    actions_to_run = std::move(m_deferred_commander_actions);
                }
                for (auto& action : actions_to_run)
                {
                    if (action)
                    {
                        action();
                    }
                }
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_deferred_commander_actions.clear();
            }

            m_commander_active.store(commander_active, std::memory_order_relaxed);

            adam::log incoming_log;
            bool any_logs = false;
            if (log_sink_active)
            {
                std::vector<log_entry> local_logs;
                if (m_running.load(std::memory_order_relaxed) && m_log_sink.queue().pop(incoming_log, 50))
                {
                    local_logs.push_back({ incoming_log.get_timestamp(), incoming_log.get_level(), std::string(incoming_log.get_text()) });

                    while (m_running.load(std::memory_order_relaxed) && m_log_sink.queue().pop(incoming_log, 0))
                    {
                        local_logs.push_back({ incoming_log.get_timestamp(), incoming_log.get_level(), std::string(incoming_log.get_text()) });
                    }
                }

                if (!local_logs.empty())
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_log_history.insert(m_log_history.end(), local_logs.begin(), local_logs.end());
                    if (m_log_history.size() > m_max_log_history)
                    {
                        m_log_history.erase(m_log_history.begin(), m_log_history.begin() + (m_log_history.size() - m_max_log_history));
                    }
                    any_logs = true;
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            if (any_logs)
            {
                request_redraw();
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_radar_mutex);
            for (auto& [key, inspector] : m_radar_inspectors)
            {
                if (!inspector)
                {
                    continue;
                }

                if (key.is_input)
                {
                    m_commander.request_connection_input_inspector_destroy(inspector);
                }
                else
                {
                    m_commander.request_connection_output_inspector_destroy(inspector);
                }
            }
            m_radar_inspectors.clear();
        }

        m_log_sink.destroy();
        m_commander.destroy();
    }

    void cop_controller::handle_radar_data(adam::string_hash conn_hash, bool is_input, const uint8_t* data, size_t size, uint64_t timestamp)
    {
        if (!data || size == 0)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_radar_mutex);
            auto& stats = m_radar_stats[{ conn_hash, is_input }];
            stats.msg_count++;
            stats.total_bytes += size;
            stats.last_timestamp = timestamp;
            format_stream_preview_hex(data, size, stats.last_preview_hex, sizeof(stats.last_preview_hex));
        }

        // ASTERIX data inspection: Auto-detect radar sites & CAT034 North Marker coordinates
        if (size >= 4)
        {
            uint8_t cat = data[0];
            uint16_t len = (static_cast<uint16_t>(data[1]) << 8) | data[2];
            if (len >= 4 && len <= size)
            {
                size_t offset = 3;
                size_t fspec_len = 0;
                while (offset + fspec_len < len)
                {
                    uint8_t fbyte = data[offset + fspec_len];
                    fspec_len++;
                    if ((fbyte & 0x01) == 0)
                    {
                        break;
                    }
                }

                if (offset + fspec_len + 2 <= len)
                {
                    uint8_t first_fspec = data[offset];
                    if ((first_fspec & 0x80) != 0)
                    {
                        uint8_t sac = data[offset + fspec_len];
                        uint8_t sic = data[offset + fspec_len + 1];

                        if (is_auto_detect_sites())
                        {
                            bool site_exists = false;
                            for (const auto& s : m_sites)
                            {
                                if (s && s->get_sac() == sac && s->get_sic() == sic)
                                {
                                    site_exists = true;
                                    break;
                                }
                            }

                            if (!site_exists)
                            {
                                char site_name[64];
                                snprintf(site_name, sizeof(site_name), "Site %u/%u", sac, sic);
                                auto new_site = std::make_unique<site>(adam::string_hashed(&site_name[0]));
                                new_site->set_sac(sac);
                                new_site->set_sic(sic);
                                new_site->set_label(adam::string_hashed(&site_name[0]));
                                new_site->set_auto_retrieve_coords(true);
                                new_site->set_auto_calc_range(true);
                                m_sites.push_back(std::move(new_site));
                                sync_sites_to_config();
                                save_config();
                            }
                        }

                        // CAT034 North Marker Sensor Position in WGS-84 (Item 030)
                        if (cat == 34)
                        {
                            size_t item_offset = offset + fspec_len + 2;
                            if ((first_fspec & 0x40) != 0 && item_offset < len)
                            {
                                item_offset += 1;
                            }

                            if ((first_fspec & 0x20) != 0 && item_offset + 6 <= len)
                            {
                                int32_t raw_lat = (static_cast<int32_t>(data[item_offset]) << 24) |
                                                  (static_cast<int32_t>(data[item_offset + 1]) << 16) |
                                                  (static_cast<int32_t>(data[item_offset + 2]) << 8);
                                raw_lat >>= 8;

                                int32_t raw_lon = (static_cast<int32_t>(data[item_offset + 3]) << 24) |
                                                  (static_cast<int32_t>(data[item_offset + 4]) << 16) |
                                                  (static_cast<int32_t>(data[item_offset + 5]) << 8);
                                raw_lon >>= 8;

                                double decoded_lat = static_cast<double>(raw_lat) * (180.0 / 8388608.0);
                                double decoded_lon = static_cast<double>(raw_lon) * (180.0 / 8388608.0);

                                for (auto& s : m_sites)
                                {
                                    if (s && s->get_sac() == sac && s->get_sic() == sic && s->get_auto_retrieve_coords())
                                    {
                                        s->set_lat(static_cast<float>(decoded_lat));
                                        s->set_lon(static_cast<float>(decoded_lon));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        radar_data_callback cb;
        {
            std::lock_guard<std::mutex> lock(m_radar_mutex);
            cb = m_radar_data_callback;
        }

        if (cb)
        {
            cb(conn_hash, is_input, data, size, timestamp);
        }

        request_redraw();
    }

    void cop_controller::set_radar_data_callback(radar_data_callback cb)
    {
        std::lock_guard<std::mutex> lock(m_radar_mutex);
        m_radar_data_callback = std::move(cb);
    }

    void cop_controller::set_radar_stream_enabled(adam::string_hash conn_hash, bool is_input, bool enable)
    {
        radar_stream_key key{ conn_hash, is_input };

        if (enable)
        {
            enqueue_commander_action([this, conn_hash, is_input, key]()
            {
                {
                    std::lock_guard<std::mutex> lock(m_radar_mutex);
                    if (m_radar_inspectors.find(key) != m_radar_inspectors.end())
                    {
                        return;
                    }
                }

                adam::data_inspector* new_inspector = nullptr;
                auto callback = [this, conn_hash, is_input](adam::buffer* buf)
                {
                    if (!buf)
                    {
                        return;
                    }

                    const uint8_t* data_ptr = buf->get_begin_as<uint8_t>();
                    size_t data_size = buf->get_size();
                    uint64_t ts = buf->get_timestamp();
                    if (ts == 0)
                    {
                        ts = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
                    }

                    this->handle_radar_data(conn_hash, is_input, data_ptr, data_size, ts);
                };

                adam::response_status status = adam::response_status::failed;
                if (is_input)
                {
                    status = m_commander.request_connection_input_inspector_create(conn_hash, callback, new_inspector);
                }
                else
                {
                    status = m_commander.request_connection_output_inspector_create(conn_hash, callback, new_inspector);
                }

                if (status == adam::response_status::success && new_inspector)
                {
                    std::lock_guard<std::mutex> lock(m_radar_mutex);
                    m_radar_inspectors[key] = new_inspector;
                }
            });
        }
        else
        {
            enqueue_commander_action([this, conn_hash, is_input, key]()
            {
                adam::data_inspector* inspector_to_destroy = nullptr;
                {
                    std::lock_guard<std::mutex> lock(m_radar_mutex);
                    auto it = m_radar_inspectors.find(key);
                    if (it != m_radar_inspectors.end())
                    {
                        inspector_to_destroy = it->second;
                        m_radar_inspectors.erase(it);
                    }
                }

                if (!inspector_to_destroy)
                {
                    return;
                }

                if (is_input)
                {
                    m_commander.request_connection_input_inspector_destroy(inspector_to_destroy);
                }
                else
                {
                    m_commander.request_connection_output_inspector_destroy(inspector_to_destroy);
                }
            });
        }
    }

    bool cop_controller::is_radar_stream_enabled(adam::string_hash conn_hash, bool is_input) const
    {
        std::lock_guard<std::mutex> lock(m_radar_mutex);
        return m_radar_inspectors.find({ conn_hash, is_input }) != m_radar_inspectors.end();
    }

    bool cop_controller::get_radar_stream_stats(adam::string_hash conn_hash, bool is_input, radar_stream_stats& out_stats) const
    {
        std::lock_guard<std::mutex> lock(m_radar_mutex);
        auto it = m_radar_stats.find({ conn_hash, is_input });
        if (it == m_radar_stats.end())
        {
            out_stats = radar_stream_stats();
            return false;
        }

        out_stats = it->second;
        return true;
    }

    void cop_controller::clear_radar_stream_stats()
    {
        std::lock_guard<std::mutex> lock(m_radar_mutex);
        m_radar_stats.clear();
        request_redraw();
    }

    void cop_controller::enable_all_radar_streams(bool enable)
    {
        std::vector<std::pair<adam::string_hash, bool>> matching_endpoints;
        {
            std::lock_guard<const adam::registry_view> reg_lock(m_commander.registry());
            for (const auto& [conn_hash, conn] : m_commander.registry().get_connections())
            {
                if (!conn)
                {
                    continue;
                }

                if (conn->input_format.get_hash() == "asterix"_ct.get_hash())
                {
                    matching_endpoints.emplace_back(conn_hash, true);
                }

                if (conn->output_format.get_hash() == "asterix"_ct.get_hash())
                {
                    matching_endpoints.emplace_back(conn_hash, false);
                }
            }
        }

        for (const auto& [conn_hash, is_input] : matching_endpoints)
        {
            set_radar_stream_enabled(conn_hash, is_input, enable);
        }
    }
}
