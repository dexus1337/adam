#pragma once

/**
 * @file    connection.hpp
 * @author  dexus1337
 * @brief   Defines a data connection between n+ inputs ports and n+ output ports
 * @version 1.0
 * @date    07.08.2026
 */

 
#include "api/api.hpp"
#include "configuration/configuration-item.hpp"

#include <vector>

namespace adam 
{
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

        /** @brief Constructs a new connection object. */
        connection(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the connection object and cleans up resources. */
        ~connection();

    protected:

        std::vector<std::unique_ptr<port_input>>     m_input_ports;
        std::vector<std::unique_ptr<data_processor>> m_processors;
        std::vector<std::unique_ptr<port_output>>    m_output_ports;

    };
}