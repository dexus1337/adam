#pragma once

/**
 * @file    port-output.hpp
 * @author  dexus1337
 * @brief   Defines an output port for sending data in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "data/port/port.hpp"


namespace adam 
{
    /**
     * @class port_output
     * @brief An output port for sending data in the ADAM system.
     */
    class ADAM_SDK_API port_output : public port
    {
    public:

        /** @brief Constructs a new input port object. */
        port_output(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_output();
    };
}