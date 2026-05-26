#pragma once

/**
 * @file    registry-view.hpp
 * @author  dexus1337
 * @brief   Defines views for controller registry elements.
 * @version 1.0
 * @date    13.05.2026
 */

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "data/port/port.hpp"
#include "memory/buffer/buffer.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef ADAM_CPU_X64
#include <immintrin.h>
#endif

namespace adam 
{
    /** @struct port_view */
    struct port_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed type_module;
        string_hashed datatype;
        string_hashed datatype_module;
        bool is_active              = false;
        bool is_unavailable         = false;
        port::direction direction    = port::direction_invalid;
        buffer* statistic_buffer    = nullptr;

        ~port_view() { if (statistic_buffer) statistic_buffer->release(); }
    };

    /** @struct filter_view */
    struct filter_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
    };

    /** @struct converter_view */
    struct converter_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
    };

    /** @struct connection_view */
    struct connection_view
    {
        string_hashed name;
        std::vector<string_hash> inputs;
        std::vector<string_hash> outputs;
        std::vector<string_hash> filters;
        std::vector<string_hash> converters;
        bool is_active = false;

        uint64_t created = 0;
        uint64_t edited = 0;
        uint32_t sorting_index = 0;
        uint32_t color = 0;
    };

    /**
     * @class registry_view
     * @brief Holds a local view of the controller's registry elements.
     */
    class ADAM_SDK_API registry_view
    {
    public:
        using port_map       = std::unordered_map<string_hash, std::unique_ptr<port_view>>;
        using filter_map     = std::unordered_map<string_hash, std::unique_ptr<filter_view>>;
        using converter_map  = std::unordered_map<string_hash, std::unique_ptr<converter_view>>;
        using connection_map = std::unordered_map<string_hash, std::unique_ptr<connection_view>>;

        port_map&       ports()         { return m_ports; }
        filter_map&     filters()       { return m_filters; }
        converter_map&  converters()    { return m_converters; }
        connection_map& connections()   { return m_connections; }

        const port_map&       get_ports() const         { return m_ports; }
        const filter_map&     get_filters() const       { return m_filters; }
        const converter_map&  get_converters() const    { return m_converters; }
        const connection_map& get_connections() const   { return m_connections; }

        void lock() const
        {
            while (m_lock.test_and_set(std::memory_order_acquire))
            {
                while (m_lock.test(std::memory_order_relaxed))
                {
                    #ifdef ADAM_CPU_X64
                    _mm_pause(); // Hardware pause (no OS yield) to reduce power and memory bus saturation
                    #endif
                }
            }
        }

        void unlock() const { m_lock.clear(std::memory_order_release); }

        void clear()
        {
            std::lock_guard<const registry_view> lg(*this);
            m_ports.clear();
            m_filters.clear();
            m_converters.clear();
            m_connections.clear();
        }

    private:
        mutable std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
        port_map       m_ports;
        filter_map     m_filters;
        converter_map  m_converters;
        connection_map m_connections;
    };
}