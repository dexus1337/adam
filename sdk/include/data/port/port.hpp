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

        static constexpr size_t max_name_length = 64;
        static constexpr size_t max_type_length = 64;
        static constexpr size_t max_module_name_length = 64;

        struct basic_info
        {
            char name[max_name_length];
            char type[max_type_length];
            char module_name[max_module_name_length];

            void setup(const string_hashed& n, const string_hashed& t, const string_hashed& m = string_hashed())
            {
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                std::strncpy(type, t.c_str(), sizeof(type) - 1);
                type[sizeof(type) - 1] = '\0';
                std::strncpy(module_name, m.c_str(), sizeof(module_name) - 1);
                module_name[sizeof(module_name) - 1] = '\0';
            }
        };
        static_assert(sizeof(port::basic_info) <= command::get_max_data_length(), "port::basic_info exceeds maximum command data size");


        /** @brief Retrieves the default configuration parameters for ports. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Destroys the port object and cleans up resources. */
        virtual ~port();

        virtual const string_hashed_ct& get_type_name() const = 0;

        const data_format* get_data_format() const { return m_data_format; }

        vector_double_buffer<connection*>&                      connections()   { return m_connections; }
        vector_double_buffer<std::shared_ptr<data_inspector>>&  inspectors()    { return m_inspectors; }

        /** @brief Data management routine */
        virtual bool handle_data(buffer* buffer);

    protected:

        /** @brief Constructs a new port object. */
        port(const string_hashed& item_name);

        const data_format* m_data_format;                                       /**< The data format associated with this port, used for parsing/serializing data. */

        vector_double_buffer<connection*>                       m_connections;  /**< Each port needs to know where to send/recieve data from */
        vector_double_buffer<std::shared_ptr<data_inspector>>   m_inspectors;   /**< Zero or many data inspectors. All incoming data will be forwarded to them */

    };
}