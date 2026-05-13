#pragma once

/**
 * @file    connection.hpp
 * @author  dexus1337
 * @brief   Defines a data connection between n+ inputs ports and n+ output ports
 * @version 1.0
 * @date    07.08.2026
 */

 
#include "api/sdk-api.hpp"
#include "configuration/configuration-item.hpp"
#include "types/vector-double-buffer.hpp"
#include "commander/messages/command.hpp"

#include <cstring>


namespace adam 
{
    class buffer;
    class port_input;
    class data_processor;
    class port_output;

    /**
     * @class   connection
     * @brief   Defines a data connection between n+ inputs ports and n+ output ports
     */
    class ADAM_SDK_API connection : public configuration_item
    {
    public:

        static constexpr size_t max_name_length = 64;

        struct basic_info
        {
            char name[max_name_length];

            void setup(const string_hashed& n)
            {
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }
        };
        static_assert(sizeof(connection::basic_info) <= command::get_max_data_length(), "connection::basic_info exceeds maximum command data size");

        /** @brief Constructs a new connection object. */
        connection(const string_hashed& item_name);

        /** @brief Destroys the connection object and cleans up resources. */
        ~connection();

        vector_double_buffer<port_input*>&      ports_input()   { return m_ports_input; }
        vector_double_buffer<data_processor*>&  processors()    { return m_processors; }
        vector_double_buffer<port_output*>&     ports_output()  { return m_ports_output; }

        /** @brief Data input routine. Data arrives here, gets passed through processors and then to output ports */
        bool handle_data(buffer* buffer);

    protected:

        vector_double_buffer<port_input*>     m_ports_input;
        vector_double_buffer<data_processor*> m_processors;
        vector_double_buffer<port_output*>    m_ports_output;

    };
}