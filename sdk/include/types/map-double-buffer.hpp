#pragma once

/**
 * @file    map-double-buffer.hpp
 * @author  dexus1337
 * @brief   A thread-safe double-buffered map for lock-free reads with atomic updates.
 * @version 1.0
 * @date    14.06.2026
 */

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace adam
{
    /**
     * @class map_double_buffer
     * @brief A generic double-buffered map that allows lock-free read access on the active buffer
     *        while safely handling concurrent modifications from the write side via updates.
     */
    template<typename key, typename value>
    class map_double_buffer
    {
    public:
        map_double_buffer() : m_dirty(false) {}
        ~map_double_buffer() = default;

        void update(const std::unordered_map<key, value>& new_map)
        {
            {
                std::unique_lock lock(m_mutex);
                m_pending = new_map;
            }
            m_dirty.store(true, std::memory_order_release);
        }

        bool is_dirty() const
        {
            return m_dirty.load(std::memory_order_acquire);
        }

        const std::unordered_map<key, value>& get_active() const
        {
            if (m_dirty.load(std::memory_order_acquire))
            {
                std::unique_lock lock(m_mutex);
                const_cast<std::unordered_map<key, value>&>(m_active) = m_pending;
                const_cast<std::atomic<bool>&>(m_dirty).store(false, std::memory_order_release);
            }
            return m_active;
        }

    private:
        mutable std::unordered_map<key, value> m_active;
        mutable std::unordered_map<key, value> m_pending;
        mutable std::shared_mutex m_mutex;
        mutable std::atomic<bool> m_dirty;
    };
}
