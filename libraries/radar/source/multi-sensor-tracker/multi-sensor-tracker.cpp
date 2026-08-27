#include "multi-sensor-tracker/multi-sensor-tracker.hpp"
#include <algorithm>

namespace adam::lib::radar
{
    multi_sensor_tracker::multi_sensor_tracker()
        : m_sites_table(std::make_unique<std::atomic<site*>[]>(65536))
    {
        for (size_t i = 0; i < 65536; ++i)
        {
            m_sites_table[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    site* multi_sensor_tracker::get_site(adam::string_hash name_hash) const
    {
        const auto& map = m_buf_by_name.get();
        auto it = map.find(name_hash);
        if (it == map.end())
        {
            return nullptr;
        }

        return it->second;
    }

    bool multi_sensor_tracker::has_site(adam::string_hash name_hash) const
    {
        const auto& map = m_buf_by_name.get();
        return map.find(name_hash) != map.end();
    }

    std::vector<site*> multi_sensor_tracker::get_sites() const
    {
        std::vector<site*> result;
        m_buf_list.iterate([&result](const auto& active)
        {
            result = active;
        });
        return result;
    }

    void multi_sensor_tracker::sync_buffers()
    {
        m_buf_by_name.update(m_sites_by_name);
        m_buf_list.reorder(m_sites_list);
    }

    void multi_sensor_tracker::register_site(site* s)
    {
        if (!s)
        {
            return;
        }

        adam::spinlock::guard lock(m_write_lock);

        uint16_t key = make_sacsic_key(s->get_sacsic());
        adam::string_hash name_hash = s->get_name().get_hash();

        m_sites_table[key].store(s, std::memory_order_release);

        auto it = std::find(m_sites_list.begin(), m_sites_list.end(), s);
        if (it == m_sites_list.end())
        {
            m_sites_list.push_back(s);
        }

        m_sites_by_name[name_hash] = s;

        sync_buffers();
    }

    void multi_sensor_tracker::unregister_site(site* s)
    {
        if (!s)
        {
            return;
        }

        adam::spinlock::guard lock(m_write_lock);

        uint16_t key = make_sacsic_key(s->get_sacsic());
        m_sites_table[key].store(nullptr, std::memory_order_release);

        auto it = std::find(m_sites_list.begin(), m_sites_list.end(), s);
        if (it != m_sites_list.end())
        {
            m_sites_list.erase(it);
        }

        m_sites_by_name.erase(s->get_name().get_hash());

        sync_buffers();
    }

    void multi_sensor_tracker::unregister_site(uint8_t sac, uint8_t sic)
    {
        unregister_site(make_sacsic_key(sac, sic));
    }

    void multi_sensor_tracker::unregister_site(uint16_t sacsic_key)
    {
        adam::spinlock::guard lock(m_write_lock);
        site* s = m_sites_table[sacsic_key].load(std::memory_order_relaxed);
        if (!s)
        {
            return;
        }

        m_sites_table[sacsic_key].store(nullptr, std::memory_order_release);
        m_sites_by_name.erase(s->get_name().get_hash());

        auto list_it = std::find(m_sites_list.begin(), m_sites_list.end(), s);
        if (list_it != m_sites_list.end())
        {
            m_sites_list.erase(list_it);
        }

        sync_buffers();
    }

    void multi_sensor_tracker::unregister_site(adam::string_hash name_hash)
    {
        adam::spinlock::guard lock(m_write_lock);
        auto it = m_sites_by_name.find(name_hash);
        if (it == m_sites_by_name.end())
        {
            return;
        }

        site* s = it->second;
        m_sites_by_name.erase(it);

        if (s)
        {
            m_sites_table[make_sacsic_key(s->get_sacsic())].store(nullptr, std::memory_order_release);
            auto list_it = std::find(m_sites_list.begin(), m_sites_list.end(), s);
            if (list_it != m_sites_list.end())
            {
                m_sites_list.erase(list_it);
            }
        }

        sync_buffers();
    }

    void multi_sensor_tracker::clear_sites()
    {
        adam::spinlock::guard lock(m_write_lock);
        for (size_t i = 0; i < 65536; ++i)
        {
            m_sites_table[i].store(nullptr, std::memory_order_release);
        }
        m_sites_list.clear();
        m_sites_by_name.clear();

        sync_buffers();
    }
}
