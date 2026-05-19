#pragma once

/**
 * @file    connection.hpp
 * @author  dexus1337
 * @brief   Defines a data connection between n+ inputs ports and n+ output ports
 * @version 1.0
 * @date    07.08.2026
 */

 
#include "api/api-sdk.hpp"
#include "configuration/configuration-item.hpp"
#include "types/vector-double-buffer.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"

#include <cstring>


namespace adam 
{
    class buffer;
    class data_processor;
    class port;

    /**
     * @class   connection
     * @brief   Defines a data connection between n+ inputs ports and n+ output ports
     */
    class ADAM_SDK_API connection : public configuration_item
    {
    public:

        struct basic_info
        {
            char name[max_name_length];

            void setup(const string_hashed& n)
            {
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }

            uint16_t input_count;
            uint16_t processor_count;
            uint16_t output_count;

            // TODO: add timestamp created, edited, sortingidx, color. ALSO FOR PORTS

            static constexpr size_t default_type_count = ((command::get_max_data_length() - sizeof(name)) / 3) / sizeof(string_hashed::hash_datatype);

            string_hashed::hash_datatype inputs[default_type_count];
            string_hashed::hash_datatype processors[default_type_count];
            string_hashed::hash_datatype outputs[default_type_count];
        };
        static_assert(sizeof(connection::basic_info) <= command::get_max_data_length(), "connection::basic_info exceeds maximum command data size");

        /** @brief Retrieves the default configuration parameters for ports. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new connection object. */
        connection(const string_hashed& item_name);

        /** @brief Destroys the connection object and cleans up resources. */
        ~connection();

        vector_double_buffer<port*>&            ports_input()   { return m_ports_input; }
        vector_double_buffer<data_processor*>&  processors()    { return m_processors; }
        vector_double_buffer<port*>&            ports_output()  { return m_ports_output; }
        
        std::vector<string_hashed>& unavailable_inputs()        { return m_unavailable_inputs; }
        std::vector<string_hashed>& unavailable_outputs()       { return m_unavailable_outputs; }

        /** @brief Data input routine. Data arrives here, gets passed through processors and then to output ports */
        bool handle_data(buffer* buffer);

        /** @brief Starts the connection. */
        bool start();

        /** @brief Stops the connection. */
        bool stop();

    protected:

        vector_double_buffer<port*>           m_ports_input;
        vector_double_buffer<data_processor*> m_processors;
        vector_double_buffer<port*>           m_ports_output;
        std::vector<string_hashed>            m_unavailable_inputs;
        std::vector<string_hashed>            m_unavailable_outputs;

    };
}