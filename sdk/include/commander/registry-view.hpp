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
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>

#include <algorithm>
#include <cstring>
#include "configuration/parameters/configuration-parameter.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "commander/messages/message-serializer.hpp"

#ifdef ADAM_CPU_X64
#include <immintrin.h>
#endif

namespace adam 
{
    namespace detail
    {

        template<typename MessageType>
        void deserialize_user_parameters(uint16_t count, message_deserializer<MessageType>& deserializer, configuration_parameter_list& user_params) 
        {
            for (uint16_t i = 0; i < count; ++i)
            {
                auto header = deserializer.template read_view<configuration_parameter::view>();
                
                switch (header.var_type)
                {
                    case configuration_parameter::type::type_boolean: { bool value; deserializer.read_bytes(&value, sizeof(bool)); if (auto* p = user_params.get<configuration_parameter_boolean>(header.name)) p->set_value(value); break; }
                    case configuration_parameter::type::type_integer: { int64_t value; deserializer.read_bytes(&value, sizeof(int64_t)); if (auto* p = user_params.get<configuration_parameter_integer>(header.name)) p->set_value(value); break; }
                    case configuration_parameter::type::type_double:  { double value; deserializer.read_bytes(&value, sizeof(double)); if (auto* p = user_params.get<configuration_parameter_double>(header.name)) p->set_value(value); break; }
                    case configuration_parameter::type::type_string:  {
                        uint16_t length; deserializer.read_bytes(&length, sizeof(uint16_t)); std::string value(length, '\0'); if (length > 0) deserializer.read_bytes(value.data(), length);
                        if (auto* p = user_params.get<configuration_parameter_string>(header.name)) p->set_value(string_hashed(value));
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    /** @struct port_view */
    struct port_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed type_module;
        bool started;
        bool is_unavailable;
        port::direction direction;
        buffer* statistic_buffer;
        configuration_parameter_list_sorted user_params{string_hashed("user_parameters")};
        
        ~port_view() { if (statistic_buffer) statistic_buffer->release(); }
    };

    /** @struct processor_view */
    struct processor_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
        bool is_filter;
        bool is_unavailable;
        string_hashed input_datatype;
        string_hashed output_datatype;
        buffer* state_buffer;
        configuration_parameter_list_sorted user_params{string_hashed("user_parameters")};

        ~processor_view() { if (state_buffer) state_buffer->release(); }
    };

    /** @struct connection_view */
    struct connection_view
    {
        string_hashed name;
        std::vector<string_hash> inputs;
        std::vector<string_hash> outputs;
        std::vector<string_hash> processors;
        bool started;
        bool valid_chain;
        bool is_unavailable;

        uint64_t created;
        uint64_t edited;
        uint32_t sorting_index;
        uint32_t color;

        /** @brief Input and output data formats of this connection. */
        string_hashed input_format;
        string_hashed input_format_module;
        string_hashed output_format;
        string_hashed output_format_module;
    };

    /**
     * @class registry_view
     * @brief Holds a local view of the controller's registry elements.
     */
    class ADAM_SDK_API registry_view
    {
    public:
        using port_map       = std::unordered_map<string_hash, std::unique_ptr<port_view>>;
        using processor_map  = std::unordered_map<string_hash, std::unique_ptr<processor_view>>;
        using connection_map = std::unordered_map<string_hash, std::unique_ptr<connection_view>>;

        port_map&           ports()             { return m_ports; }
        processor_map&      processors()        { return m_processors; }
        connection_map&     connections()       { return m_connections; }

        const port_map&             get_ports() const           { return m_ports; }
        const processor_map&        get_processors() const      { return m_processors; }
        const connection_map&       get_connections() const     { return m_connections; }

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
            m_processors.clear();
            m_connections.clear();
        }

    private:
        mutable std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
        port_map       m_ports;
        processor_map  m_processors;
        connection_map m_connections;
    };
}