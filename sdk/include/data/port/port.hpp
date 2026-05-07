#pragma once

/**
 * @file    port.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, providing a common interface for handling data flow in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "configuration/configuration-item.hpp"

namespace adam 
{
    class data_format;

    /**
     * @class port
     * @brief A base class for ports, providing a common interface for handling data flow in the ADAM system.
     */
    class ADAM_SDK_API port : public configuration_item
    {
    public:

    protected:

        /** @brief Constructs a new port object. */
        port(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the port object and cleans up resources. */
        ~port();

        const data_format* m_data_format; /**< The data format associated with this port, used for parsing/serializing data. */

    };
}