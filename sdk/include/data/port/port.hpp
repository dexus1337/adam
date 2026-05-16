#pragma once

/**
 * @file    port.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, providing a common interface for handling data flow in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"
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
            string_hashed::hash_datatype type;
            string_hashed::hash_datatype module;

            void setup(const string_hashed& n, string_hashed::hash_datatype t, string_hashed::hash_datatype m)
            {
                type = t;
                module = m;
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }
        };
        static_assert(sizeof(port::basic_info) <= command::get_max_data_length(), "port::basic_info exceeds maximum command data size");

        struct statistic_info
        {
            bool is_active;
            uint64_t total_buffers_handled;
            uint64_t total_bytes_handled;
        };

        static constexpr size_t statistic_info_buffer_size = sizeof(statistic_info);


        /** @brief Retrieves the default configuration parameters for ports. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Destroys the port object and cleans up resources. */
        virtual ~port();

        virtual const string_hashed_ct& get_type_name() const = 0;

        const data_format* get_data_format() const { return m_data_format; }

        vector_double_buffer<connection*>&                      connections()   { return m_connections; }
        vector_double_buffer<std::shared_ptr<data_inspector>>&  inspectors()    { return m_inspectors; }

        virtual bool is_active() const { return m_statistic_buffer->get_data_as<statistic_info>()->is_active; }

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
    };
}