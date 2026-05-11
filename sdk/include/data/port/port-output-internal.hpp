#pragma once

/**
 * @file    port-output-internal.hpp
 * @author  dexus1337
 * @brief   Defines an output port that allows passing data directly to another input port (port_intput_internal)
 * @version 1.0
 * @date    11.05.2026
 */

 
#include "api/sdk-api.hpp"
#include "data/port/port-output.hpp"


namespace adam 
{
    class port_input_internal;

    /**
     * @class port_output_internal
     * @brief   Defines an output port that allows passing data directly to another input port (port_intput_internal)
     */
    class ADAM_SDK_API port_output_internal : public port_output
    {
    public:

        static constexpr string_hashed_ct type_name = string_hashed_ct("output_internal");

        /** @brief Constructs a new output port object. */
        port_output_internal(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the output port object and cleans up resources. */
        ~port_output_internal();

        virtual constexpr const string_hashed_ct& get_type_name() const override { return type_name; }

    protected:

        vector_double_buffer<port_input_internal*> m_connected_inputs;
    };
}