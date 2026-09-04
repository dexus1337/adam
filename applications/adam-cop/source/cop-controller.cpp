/**
 * @file    cop-controller.cpp
 * @author  dexus1337
 * @brief   Application state controller implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "cop-controller.hpp"
#include <fstream>
#include <data/asterix-internal.hpp>
#include <data/asterix-types.hpp>
#include <data/categories/034/cat034-structs.hpp>

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
        load();
    }

    cop_controller::~cop_controller()
    {
        save();
        stop();
    }

    void cop_controller::start()
    {
        if (m_running.exchange(true)) return;

        m_worker_thread = std::thread(&cop_controller::update_loop, this);
    }

    void cop_controller::stop()
    {
        if (!m_running.exchange(false)) return;

        if (m_worker_thread.joinable())
            m_worker_thread.join();
    }

    bool cop_controller::is_commander_active() const
    {
        return m_commander_active.load();
    }

    std::vector<log_entry> cop_controller::get_log_history() const
    {
        adam::spinlock::guard lock(m_lock);
        return m_log_history;
    }

    bool cop_controller::is_log_history_empty() const
    {
        adam::spinlock::guard lock(m_lock);
        return m_log_history.empty();
    }

    void cop_controller::clear_log_history()
    {
        adam::spinlock::guard lock(m_lock);
        m_log_history.clear();
    }

    void cop_controller::request_redraw()
    {
        if (!m_redraw_callback) return;

        m_redraw_callback();
    }

    void cop_controller::set_redraw_callback(std::function<void()> cb)
    {
        m_redraw_callback = std::move(cb);
    }

    void cop_controller::enqueue_commander_action(std::function<void()> action)
    {
        if (!action) return;

        adam::spinlock::guard lock(m_lock);
        m_deferred_commander_actions.push_back(std::move(action));
    }

    void cop_controller::add_waypoint(std::unique_ptr<waypoint> wp)
    {
        if (!wp) return;

        m_waypoints.push_back(std::move(wp));
        save();
        request_redraw();
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
            save();
            request_redraw();
        }
    }

    void cop_controller::clear_waypoints()
    {
        m_waypoints.clear();
        save();
        request_redraw();
    }

    void cop_controller::add_site(std::unique_ptr<drawable_site> s)
    {
        if (!s) return;

        m_tracker.register_site(s.get());
        m_sites.push_back(std::move(s));
        save();
        request_redraw();
    }

    void cop_controller::remove_site(adam::string_hash item_hash)
    {
        auto it = std::find_if(m_sites.begin(), m_sites.end(), [item_hash](const std::unique_ptr<drawable_site>& s) 
        { 
            return s->get_name().get_hash() == item_hash; 
        });

        if (it != m_sites.end())
        {
            m_tracker.unregister_site(it->get());
            m_sites.erase(it);
            save();
            request_redraw();
        }
    }

    void cop_controller::clear_sites()
    {
        m_tracker.clear_sites();
        m_sites.clear();
        save();
        request_redraw();
    }

    bool cop_controller::is_auto_detect_sites() const
    {
        return m_p_auto_detect_sites->get_value();
    }

    void cop_controller::set_auto_detect_sites(bool enable)
    {
        m_p_auto_detect_sites->set_value(enable);

        save();
        request_redraw();
    }

    void cop_controller::auto_detect_sites_from_streams()
    {
        enable_all_radar_streams(true);
        set_auto_detect_sites(true);
    }

    bool cop_controller::save(adam::string_hashed::view filepath) const
    {
        if (m_p_waypoints_list)
        {
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

        if (m_p_sites_list)
        {
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

        return configuration_item::save(filepath);
    }

    bool cop_controller::load(adam::string_hashed::view filepath)
    {
        std::ifstream ifs(std::string(filepath), std::ios::binary);
        if (!ifs) return false;

        uint32_t magic = 0;
        configuration_parameter::read_binary(ifs, magic);
        if (magic != 0xadacf116) return false;

        version_info ver;
        configuration_parameter::read_binary(ifs, ver);
        uint32_t loaded_version = make_version(ver.major, ver.minor, ver.patch);
        if (get_major(loaded_version) > get_major(core_version)) return false;

        auto loaded_root = configuration_parameter::deserialize(ifs);
        if (!loaded_root || loaded_root->get_type() != configuration_parameter::type_list) return false;

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());
        m_parameters.copy_values_from(root_list);

        // Load waypoints
        m_waypoints.clear();
        if (auto* src_waypoints = root_list->get<configuration_parameter_list>("waypoints"_ct))
        {
            m_p_waypoints_list->copy_from(src_waypoints);

            for (const auto& [name, p] : src_waypoints->get_children())
            {
                auto wp = std::make_unique<waypoint>(name);
                wp->parameters().copy_values_from(p.get());
                m_waypoints.push_back(std::move(wp));
            }
        }

        // Load sites and populate multi_sensor_tracker
        m_sites.clear();
        m_tracker.clear_sites();
        if (auto* src_sites = root_list->get<configuration_parameter_list>("sites"_ct))
        {
            m_p_sites_list->copy_from(src_sites);

            for (const auto& [name, p] : src_sites->get_children())
            {
                auto s = std::make_unique<drawable_site>(name);
                s->parameters().copy_values_from(p.get());
                m_tracker.register_site(s.get());
                m_sites.push_back(std::move(s));
            }
        }

        return !ifs.bad();
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
                if (now - last_reconnect_attempt >= std::chrono::seconds(5))
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
                            adam::spinlock::guard lock(m_radar_lock);
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
                    adam::spinlock::guard lock(m_lock);
                    actions_to_run = std::move(m_deferred_commander_actions);
                }

                for (auto& action : actions_to_run)
                    action();
            }
            else
            {
                adam::spinlock::guard lock(m_lock);
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
                        local_logs.push_back({ incoming_log.get_timestamp(), incoming_log.get_level(), std::string(incoming_log.get_text()) });
                }

                if (!local_logs.empty())
                {
                    adam::spinlock::guard lock(m_lock);

                    m_log_history.insert(m_log_history.end(), local_logs.begin(), local_logs.end());

                    if (m_log_history.size() > m_max_log_history)
                        m_log_history.erase(m_log_history.begin(), m_log_history.begin() + (m_log_history.size() - m_max_log_history));

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
            adam::spinlock::guard lock(m_radar_lock);
            for (auto& [key, inspector] : m_radar_inspectors)
                key.is_input ? m_commander.request_connection_input_inspector_destroy(inspector) : m_commander.request_connection_output_inspector_destroy(inspector);
                
            m_radar_inspectors.clear();
        }

        m_log_sink.destroy();
        m_commander.destroy();
    }

    void cop_controller::handle_radar_data(adam::string_hash conn_hash, bool is_input, adam::buffer* buf, uint64_t timestamp)
    {
        adam::buffer* ref_buf = buf->get_referenced_buffer();
        const adam::buffer* payload_buf = ref_buf ? ref_buf : buf;
        const uint8_t* raw_data_ptr = payload_buf->get_begin_as<uint8_t>();
        size_t raw_size = payload_buf->get_size();

        {
            adam::spinlock::guard lock(m_radar_lock);
            auto& stats = m_radar_stats[{ conn_hash, is_input }];
            stats.msg_count++;
            stats.total_bytes += raw_size;
            stats.last_timestamp = timestamp;

            if (raw_data_ptr && raw_size > 0)
                format_stream_preview_hex(raw_data_ptr, raw_size, stats.last_preview_hex, sizeof(stats.last_preview_hex));
        }

        const auto* root_frame = buf->get_begin_as<adam::modules::asterix::frame>();
        if (root_frame && buf->get_size() >= sizeof(adam::modules::asterix::frame))
        {
            for (const auto& blk : *root_frame)
            {
                if (blk.is_removed() || blk.category != 34) continue;

                for (const auto& rec : blk)
                {
                    if (rec.is_removed()) continue;

                    // FRN 1: I034/340 Data Source Identifier (SAC/SIC)
                    const auto* item_sacsic = rec.get_item(1);

                    if (!item_sacsic || !item_sacsic->is_populated()) continue;

                    const auto* sacsic_data = item_sacsic->get_data_as<const adam::modules::asterix::raw_sac_sic>(payload_buf);

                    uint8_t sac = sacsic_data->get_sac();
                    uint8_t sic = sacsic_data->get_sic();

                    // FRN 2: Message Type (I034/000)
                    uint8_t msg_type = 0;
                    const auto* item_msg_type = rec.get_item(2);

                    if (item_msg_type && item_msg_type->is_populated())
                        msg_type = *item_msg_type->get_data_as<const uint8_t>(payload_buf);

                    // Auto-detect radar sites on first sector crossing or north marker
                    if (is_auto_detect_sites() && !m_tracker.has_site(sac, sic))
                    {
                        char site_name[64];
                        snprintf(site_name, sizeof(site_name), "Site %u/%u", sac, sic);
                        auto new_site = std::make_unique<drawable_site>(adam::string_hashed(&site_name[0]));
                        new_site->set_sac(sac);
                        new_site->set_sic(sic);
                        new_site->set_label(adam::string_hashed(&site_name[0]));
                        new_site->set_auto_retrieve_coords(true);
                        new_site->set_auto_calc_range(true);
                        m_tracker.register_site(new_site.get());
                        m_sites.push_back(std::move(new_site));
                        save();
                    }

                    auto* s = m_tracker.get_site(sac, sic);
                    if (!s)
                    {
                        continue;
                    }

                    // FRN 4: Sector Number (I034/020)
                    const auto* item_sec = rec.get_item(4);
                    if (item_sec && item_sec->is_populated())
                    {
                        const auto* sec_data = item_sec->get_data_as<const uint8_t>(payload_buf);
                        if (sec_data)
                        {
                            uint8_t sector = *sec_data;
                            double azimuth_deg = static_cast<double>(sector) * (360.0 / 256.0);
                            s->set_current_sector(sector);
                            s->set_current_azimuth_deg(azimuth_deg);
                            s->set_last_sector_time(timestamp);
                        }
                    }
                    else if (msg_type == adam::modules::asterix::cat034::message_type_north_marker)
                    {
                        s->set_current_sector(0);
                        s->set_current_azimuth_deg(0.0);
                        s->set_last_north_marker_time(timestamp);
                    }

                    // FRN 5: Antenna Rotation Period (I034/041)
                    const auto* item_rot = rec.get_item(5);
                    if (item_rot && item_rot->is_populated())
                        s->set_rotation_period_s(item_rot->get_data_as<const adam::modules::asterix::cat034::raw_antenna_rotation_period>(payload_buf)->get_period_s());

                    // FRN 11: 3D-Position of Data Source (I034/120)
                    const auto* item_pos = rec.get_item(11);
                    if (item_pos && item_pos->is_populated() && s->get_auto_retrieve_coords())
                    {
                        const auto* pos_data = item_pos->get_data_as<const adam::modules::asterix::cat034::raw_position_3d>(payload_buf);
                        
                        s->set_lat(static_cast<float>(pos_data->get_latitude()));
                        s->set_lon(static_cast<float>(pos_data->get_longitude()));
                    }
                }
            }
        }

        radar_data_callback cb;

        {
            adam::spinlock::guard lock(m_radar_lock);
            cb = m_radar_data_callback;
        }

        if (cb)
        {
            cb(conn_hash, is_input, raw_data_ptr, raw_size, timestamp);
        }

        request_redraw();
    }

    void cop_controller::handle_radar_data(adam::string_hash conn_hash, bool is_input, const uint8_t* data, size_t size, uint64_t timestamp)
    {
        {
            adam::spinlock::guard lock(m_radar_lock);
            auto& stats = m_radar_stats[{ conn_hash, is_input }];
            stats.msg_count++;
            stats.total_bytes += size;
            stats.last_timestamp = timestamp;
            format_stream_preview_hex(data, size, stats.last_preview_hex, sizeof(stats.last_preview_hex));
        }

        radar_data_callback cb;
        {
            adam::spinlock::guard lock(m_radar_lock);
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
        adam::spinlock::guard lock(m_radar_lock);
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
                    adam::spinlock::guard lock(m_radar_lock);

                    if (m_radar_inspectors.find(key) != m_radar_inspectors.end()) return;
                }

                adam::data_inspector* new_inspector = nullptr;
                auto callback = [this, conn_hash, is_input](adam::buffer* buf) { this->handle_radar_data(conn_hash, is_input, buf, buf->get_timestamp()); };

                adam::response_status status = adam::response_status::failed;

                status = is_input ? m_commander.request_connection_input_inspector_create(conn_hash, callback, new_inspector) : m_commander.request_connection_output_inspector_create(conn_hash, callback, new_inspector);

                if (status == adam::response_status::success && new_inspector)
                {
                    adam::spinlock::guard lock(m_radar_lock);
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
                    adam::spinlock::guard lock(m_radar_lock);
                    auto it = m_radar_inspectors.find(key);
                    if (it != m_radar_inspectors.end())
                    {
                        inspector_to_destroy = it->second;
                        m_radar_inspectors.erase(it);
                    }
                }

                is_input ? m_commander.request_connection_input_inspector_destroy(inspector_to_destroy) : m_commander.request_connection_output_inspector_destroy(inspector_to_destroy);
            });
        }
    }

    bool cop_controller::is_radar_stream_enabled(adam::string_hash conn_hash, bool is_input) const
    {
        adam::spinlock::guard lock(m_radar_lock);
        return m_radar_inspectors.find({ conn_hash, is_input }) != m_radar_inspectors.end();
    }

    bool cop_controller::get_radar_stream_stats(adam::string_hash conn_hash, bool is_input, radar_stream_stats& out_stats) const
    {
        adam::spinlock::guard lock(m_radar_lock);

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
        adam::spinlock::guard lock(m_radar_lock);
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
                if (conn->input_format == "asterix"_ct)
                    matching_endpoints.emplace_back(conn_hash, true);

                if (conn->output_format == "asterix"_ct)
                    matching_endpoints.emplace_back(conn_hash, false);
            }
        }

        for (const auto& [conn_hash, is_input] : matching_endpoints)
            set_radar_stream_enabled(conn_hash, is_input, enable);
    }
}
