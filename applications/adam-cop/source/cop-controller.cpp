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
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_location"_ct, 1));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_x"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_y"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_content"_ct, 7));
            p.add(std::make_unique<configuration_parameter_integer>("fps_limit"_ct, 2)); // 60 FPS default
            p.add(std::make_unique<configuration_parameter_integer>("map_projection"_ct, 1)); // 0 = Equirectangular, 1 = Mercator
            p.add(std::make_unique<configuration_parameter_boolean>("show_grid"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_coastlines"_ct, true));
            p.add(std::make_unique<configuration_parameter_boolean>("show_land_fill"_ct, true));
            p.add(std::make_unique<configuration_parameter_integer>("base_provider"_ct, 0)); // 0 = CartoDB Dark, 1 = OSM, 2 = Satellite, 3 = Topo, 4 = Vector
            p.add(std::make_unique<configuration_parameter_double>("map_opacity"_ct, 1.0));
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
            p.add(std::make_unique<configuration_parameter_list_sorted>("waypoints"_ct));
            return p;
        }();
        return params;
    }

    cop_controller::cop_controller()
        : configuration_item("adam_cop_controller", get_default_parameters())
        , m_commander("COP"_ct)
        , m_running(false)
        , m_commander_active(false)
    {
        load("adam-cop-config.adamcopcfg");
        load_waypoints_from_config();
    }

    cop_controller::~cop_controller()
    {
        sync_waypoints_to_config();
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
        auto it = std::find_if(m_waypoints.begin(), m_waypoints.end(),
            [item_hash](const std::unique_ptr<waypoint>& wp) { return wp->get_name().get_hash() == item_hash; });
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
        save("adam-cop-config.adamcopcfg");
    }

    void cop_controller::load_waypoints_from_config()
    {
        m_waypoints.clear();
        auto* wp_list = get_parameter<configuration_parameter_list_sorted>("waypoints"_ct);
        if (!wp_list) return;

        for (const auto& hash : wp_list->get_order())
        {
            auto* p = wp_list->get<configuration_parameter_list>(hash);
            if (p)
            {
                auto wp = std::make_unique<waypoint>(p->get_name());
                // Configuration parameters clone on assignment
                wp->parameters() = *p; 
                m_waypoints.push_back(std::move(wp));
            }
        }
        
        // Ensure next id is correct by finding the max numeric suffix if possible, 
        // but for simplicity we can just make sure they have unique names when creating.
    }

    void cop_controller::sync_waypoints_to_config()
    {
        auto* wp_list = get_parameter<configuration_parameter_list_sorted>("waypoints"_ct);
        if (!wp_list) return;

        wp_list->clear();
        for (const auto& wp : m_waypoints)
        {
            auto p = wp->get_parameters().clone();
            p->set_name(wp->get_name());
            wp_list->add(std::move(p));
        }
    }

    void cop_controller::update_loop()
    {
        while (m_running.load())
        {
            std::vector<std::function<void()>> actions;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                actions.swap(m_deferred_commander_actions);
            }

            for (auto& action : actions)
            {
                if (action)
                {
                    action();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}
