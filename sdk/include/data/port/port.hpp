#pragma once

/**
 * @file    port.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, providing a common interface for handling data flow in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
#include "configuration/configuration-item.hpp"
#include "types/vector-double-buffer.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "memory/buffer/buffer.hpp"

#include <memory>
#include <cstring>

namespace adam 
{
    class buffer;
    class data_format;
    class connection;
    class data_inspector;

    /**
     * @enum    port_direction
     * @brief   Bitmask enumeration defining the supported data flow directions of a port.
     */
    enum class port_direction : uint8_t
    {
        none          = 0,
        input         = 1 << 0,
        output        = 1 << 1,
        in_out        = input | output
    };

    inline port_direction operator|(port_direction a, port_direction b) { return static_cast<port_direction>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b)); }
    inline port_direction operator&(port_direction a, port_direction b) { return static_cast<port_direction>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b)); }
    inline port_direction operator^(port_direction a, port_direction b) { return static_cast<port_direction>(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b)); }
    inline port_direction operator~(port_direction a)                   { return static_cast<port_direction>(~static_cast<uint8_t>(a)); }

    /**
     * @class port
     * @brief A base class for ports, providing a common interface for handling data flow in the ADAM system.
     * 
     *        Provides the following configuraton parameter:
     *        - type:           The type of the port, used for identification and lookup in the ADAM system. Each port implementation should have a unique type string.
     *        - data_format:    The name of the data format associated with this port
     * 
     */
    class ADAM_SDK_API port : public configuration_item
    {
    public:
        struct basic_info
        {
            char name[max_name_length];
            string_hash type;
            string_hash type_module;
            port_direction direction;
            string_hash format;
            string_hash format_module;
            bool is_active;
            bool is_unavailable;

            void setup(const string_hashed& n, string_hash t, string_hash tm, string_hash f = 0, string_hash fm = 0, bool unavail = false)
            {
                type = t;
                type_module = tm;
                direction = port_direction::none;
                format = f;
                format_module = fm;
                is_unavailable = unavail;
                is_active = false;
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }
        };
        static_assert(sizeof(port::basic_info) <= command::get_max_data_length(), "port::basic_info exceeds maximum command data size");

        struct status_event_info
        {
            string_hash     port_hash;
            buffer_handle   statistic_buffer_handle;
        };
        static_assert(sizeof(port::status_event_info) <= command::get_max_data_length(), "port::status_event_info exceeds maximum command data size");

        struct unavailable_info : public configuration_item
        {
            string_hash type;
            string_hash type_module;
            string_hash format;
            string_hash format_module;

            unavailable_info(const string_hashed& item_name)
                : configuration_item(item_name), type(0), type_module(0), format(0), format_module(0)
            {
            }
        };

        struct statistic_info
        {
            uint64_t total_buffers_handled;
            uint64_t total_bytes_handled;
            uint64_t total_buffers_discarded;
            uint64_t total_bytes_discarded;
        };

        static ADAM_CONSTEXPR size_t statistic_info_buffer_size = sizeof(statistic_info);


        /** @brief Retrieves the default configuration parameters for ports. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Destroys the port object and cleans up resources. */
        virtual ~port();

        virtual const string_hashed_ct& get_type_name() const = 0;

        /** @brief Gets the supported data flow direction capabilities of this port. */
        virtual port_direction get_direction() const = 0;

        const data_format* get_data_format() const { return m_data_format; }
        void set_data_format(const data_format* format) { m_data_format = format; }

        vector_double_buffer<connection*>&                      connections()   { return m_connections; }
        vector_double_buffer<std::shared_ptr<data_inspector>>&  inspectors()    { return m_inspectors; }

        buffer* get_statistic_buffer() const { return m_statistic_buffer; }

        virtual bool is_active() const { return m_is_active != nullptr && m_is_active->get_value(); }

        /** @brief Data management routine */
        virtual bool handle_data(buffer* buffer);

        /** @brief Starts the port. */
        virtual bool start();

        /** @brief Stops the port. */
        virtual bool stop();

    protected:

        /** @brief Constructs a new port object. */
        port(const string_hashed& item_name);

        const data_format* m_data_format;                                       /**< The data format associated with this port, used for parsing/serializing data. */

        vector_double_buffer<connection*>                       m_connections;  /**< Each port needs to know where to send/recieve data from */
        vector_double_buffer<std::shared_ptr<data_inspector>>   m_inspectors;   /**< Zero or many data inspectors. All incoming data will be forwarded to them */

        buffer* m_statistic_buffer;                                             /**< A special buffer used for storing and sharing this port's runtime statistics, such as total buffers/bytes handled and current active state. The data format of this buffer is expected to be a simple binary blob matching the structure of port::statistic_info. */

        configuration_parameter_boolean* m_is_active;                           /**< Cached pointer to the is active parameter as it will be frequently accessed. */

    };
}