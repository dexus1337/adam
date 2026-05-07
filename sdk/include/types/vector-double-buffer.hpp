#pragma once

/**
 * @file    vector-double-buffer.hpp
 * @author  dexus1337
 * @brief   A thread-safe double-buffered vector for lock-free reads with atomic updates.
 * @version 1.0
 * @date    07.05.2026
 */

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>

namespace adam
{
    /**
     * @class vector_double_buffer
     * @brief A generic double-buffered vector that allows lock-free iteration on the read side
     *        while safely handling concurrent modifications from the write side.
     * 
     * @tparam T The pointer type to store (e.g., port_input*, data_processor*)
     */
    template<typename T>
    class vector_double_buffer
    {
    public:
        /** @brief Constructs a new double buffer vector. */
        vector_double_buffer() : m_dirty(false) {}

        /** @brief Destroys the double buffer vector. */
        ~vector_double_buffer() = default;

        /**
         * @brief Adds an element to the buffer (write-side, thread-safe).
         * @param element The element to add.
         */
        void push_back(T element)
        {
            {
                std::unique_lock lock(m_mutex);
                m_pending.push_back(element);
            }
            m_dirty.store(true, std::memory_order_release);
        }

        /**
         * @brief Removes an element from the buffer (write-side, thread-safe).
         * @param element The element to remove.
         * @return True if the element was found and removed, false otherwise.
         */
        bool remove(T element)
        {
            {
                std::unique_lock lock(m_mutex);
                auto it = std::find(m_pending.begin(), m_pending.end(), element);
                if (it != m_pending.end()) {
                    m_pending.erase(it);
                } else {
                    return false;  // Element not found
                }
            }
            m_dirty.store(true, std::memory_order_release);
            return true;
        }

        /**
         * @brief Reorders the elements in the buffer.
         * @param new_order A vector with the same elements in the desired order.
         */
        void reorder(const std::vector<T>& new_order)
        {
            {
                std::unique_lock lock(m_mutex);
                m_pending = new_order;  // Copy the reordered list
            }
            m_dirty.store(true, std::memory_order_release);
        }
        
        /**
         * @brief Clears all elements from the buffer (write-side, thread-safe).
         */
        void clear()
        {
            {
                std::unique_lock lock(m_mutex);
                m_pending.clear();
            }
            m_dirty.store(true, std::memory_order_release);
        }

        /**
         * @brief Executes a function with the current stable buffer (read-side, lock-free).
         *        Automatically synchronizes pending changes if needed.
         * 
         * @param func A callable that takes const std::vector<T>& and returns void.
         * 
         * @example
         * vector_double_buffer<port_input*> ports;
         * ports.iterate([](const auto& active) {
         *     for (auto port : active) {
         *         port->process();
         *     }
         * });
         */
        template<typename Func>
        void iterate(Func func) const
        {
            // Check dirty flag (cheap atomic read, no lock)
            if (m_dirty.load(std::memory_order_acquire))
            {
                // Synchronize buffers (minimal lock time)
                std::unique_lock lock(m_mutex);
                std::swap(const_cast<std::vector<T>&>(m_active), 
                         const_cast<std::vector<T>&>(m_pending));
                const_cast<std::atomic<bool>&>(m_dirty).store(false, std::memory_order_release);
            }

            // Iterate without lock (data is stable in m_active)
            func(m_active);
        }

        /**
         * @brief Returns the current stable buffer size (read-side, approximate).
         *        Note: May not reflect pending changes until next iterate() call.
         * @return The size of the active buffer.
         */
        size_t size() const
        {
            return m_active.size();
        }

        /**
         * @brief Checks if the active buffer is empty (read-side, approximate).
         *        Note: May not reflect pending changes until next iterate() call.
         * @return True if the active buffer is empty, false otherwise.
         */
        bool empty() const
        {
            return m_active.empty();
        }

    private:
        mutable std::vector<T> m_active;   /**< The read-only buffer used during iteration. */
        mutable std::vector<T> m_pending;  /**< The write buffer for modifications. */
        mutable std::shared_mutex m_mutex; /**< Protects the pending buffer. */
        mutable std::atomic<bool> m_dirty; /**< Indicates if pending has changes to sync. */
    };
}