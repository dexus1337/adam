#pragma once

/**
 * @file    port-output-recording.hpp
 * @author  dexus1337
 * @brief   Defines an output port that allows recording data to files, providing functionality for capturing and saving data in the ADAM system.
 * @version 1.0
 * @date    11.05.2026
 */

 
#include "api/api-recrep.hpp"
 
#include <adam-sdk.hpp>


namespace adam::modules::recrep
{
    /**
     * @class port_output_recording
     * @brief Defines an output port that allows recording data to files, providing functionality for capturing and saving data in the ADAM system.
     */
    class ADAM_RECREP_API port_output_recording : public port_output
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "recording"_ct; }

        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new output port object. */
        port_output_recording(const string_hashed& item_name);

        /** @brief Destroys the output port object and cleans up resources. */
        ~port_output_recording();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };

        /** @brief Writes received data to a file. */
        virtual bool write(buffer* buff) override ;
    };
}