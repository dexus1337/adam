#pragma once

/**
 * @file    connection.hpp
 * @author  dexus1337
 * @brief   Defines a data connection between n+ input ports and n+ output ports.
 *          Each connection owns an input data format and an output data format.
 * @version 2.0
 * @date    31.05.2026
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
    class data_format;
    class data_processor;
    class port;

    /**
     * @class   connection
     * @brief   Defines a data connection between n+ input ports and n+ output ports.
     *          The connection owns the input and output data formats; individual ports
     *          carry no format information.
     */
    class ADAM_SDK_API connection : public configuration_item
    {
    public:

        struct basic_info
        {
            char name[max_name_length];

            uint64_t created;
            uint64_t edited;
            uint32_t sorting_index;
            uint32_t color;
            bool is_active;

            /** @brief Input and output formats of this connection (name hashes). */
            string_hash input_format;
            string_hash input_format_module;
            string_hash output_format;
            string_hash output_format_module;

            void setup(const string_hashed& n)
            {
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';

                created = 0;
                edited = 0;
                sorting_index = 0;
                color = 0;
                is_active = false;
                input_format = 0;
                input_format_module = 0;
                output_format = 0;
                output_format_module = 0;
            }

            uint16_t input_count;
            uint16_t processor_count;
            uint16_t output_count;

            static ADAM_CONSTEXPR size_t default_type_count = ((command::get_max_data_length() - sizeof(name) - sizeof(uint64_t) * 2 - sizeof(uint32_t) * 2 - sizeof(string_hash) * 4 - sizeof(uint16_t) * 3) / 3) / sizeof(string_hash);

            string_hash inputs[default_type_count];
            string_hash processors[default_type_count];
            string_hash outputs[default_type_count];
        };
        static_assert(sizeof(connection::basic_info) <= command::get_max_data_length(), "connection::basic_info exceeds maximum command data size");

        /** @brief Retrieves the default configuration parameters for connections. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new connection object. Formats default to transparent. */
        connection(const string_hashed& item_name);

        /** @brief Destroys the connection object and cleans up resources. */
        ~connection();

        vector_double_buffer<port*>&            ports_input()       { return m_ports_input; }
        vector_double_buffer<data_processor*>&  processors()        { return m_processors; }
        vector_double_buffer<port*>&            ports_output()      { return m_ports_output; }
        std::vector<string_hashed>&             unavailable_inputs()  { return m_unavailable_inputs; }
        std::vector<string_hashed>&             unavailable_outputs() { return m_unavailable_outputs; }

        /** @brief Returns the data format expected on all input ports. */
        const data_format* get_input_format() const  { return m_input_format; }

        /** @brief Returns the data format expected on all output ports. */
        const data_format* get_output_format() const { return m_output_format; }

        /** @brief Sets the input data format for this connection. */
        void set_input_format(const data_format* fmt)  { m_input_format = fmt; }

        /** @brief Sets the output data format for this connection. */
        void set_output_format(const data_format* fmt) { m_output_format = fmt; }

        /** @brief Data input routine. Data arrives here, gets passed through processors and then to output ports */
        bool handle_data(buffer* buffer, port* input_port = nullptr);

        /** @brief Returns true if the connection has at least one input port and one output port. */
        bool has_valid_chain();

        /** @brief Starts the connection. */
        bool start();

        /** @brief Stops the connection. */
        bool stop();

        bool is_active() const { return m_is_active != nullptr && m_is_active->get_value(); }

    protected:

        vector_double_buffer<port*>             m_ports_input;
        vector_double_buffer<data_processor*>   m_processors;
        vector_double_buffer<port*>             m_ports_output;
        std::vector<string_hashed>              m_unavailable_inputs;
        std::vector<string_hashed>              m_unavailable_outputs;

        const data_format*                      m_input_format;   /**< Format expected from all input ports (defaults to transparent). */
        const data_format*                      m_output_format;  /**< Format expected for all output ports (defaults to transparent). */
        
        configuration_parameter_boolean*        m_is_active;      /**< Cached pointer to the is_active parameter as it will be frequently accessed. */

    };
}