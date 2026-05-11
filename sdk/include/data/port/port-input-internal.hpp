#pragma once

/**
 * @file    port-input-internal.hpp
 * @author  dexus1337
 * @brief   Defines an input port that allows reading data directly from another output port (port_output_internal)
 * @version 1.0
 * @date    11.05.2026
 */

 
#include "api/sdk-api.hpp"
#include "data/port/port-input.hpp"


namespace adam 
{
    class port_output_internal;

    /**
     * @class port_input_internal
     * @brief Defines an input port that allows reading data directly from another output port (port_output_internal)
     */
    class ADAM_SDK_API port_input_internal : public port_input
    {
    public:

        static constexpr string_hashed_ct type_name = string_hashed_ct("input_internal");

        /** @brief Constructs a new input port object. */
        port_input_internal(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input_internal();

        virtual constexpr const string_hashed_ct& get_type_name() const override { return type_name; }

    protected:

        vector_double_buffer<port_output_internal*> m_connected_outputs;
    };
}