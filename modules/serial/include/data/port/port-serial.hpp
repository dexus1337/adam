#pragma once

/**
 * @file    port-serial.hpp
 * @author  dexus1337
 * @brief   Defines an serial Port. In-Out for bidirectional data transfer.
 * @version 1.0
 * @date    30.05.2026
 */


#include "api/api-serial.hpp"
 
#include <adam-sdk.hpp>


namespace adam::modules::serial
{
    /**
     * @class port_serial
     * @brief Defines an serial Port. In-Out for bidirectional data transfer.
     */
    class ADAM_SERIAL_API port_serial : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "serial"_ct; }

        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new input port object. */
        port_serial(const string_hashed& item_name);

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_serial();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }; /**< THANK YOU MSCV for your really good constexp eval. Thats why we have to do such beautiful things */
    };
}