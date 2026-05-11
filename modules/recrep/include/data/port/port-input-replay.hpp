#pragma once

/**
 * @file    port-input-replay.hpp
 * @author  dexus1337
 * @brief   Defines an input port that allows reading data from replay files, providing functionality for replaying recorded data in the ADAM system.
 * @version 1.0
 * @date    11.05.2026
 */


#include "api/recrep-api.hpp"
 
#include <adam-sdk.hpp>


namespace adam::modules::recrep
{
    /**
     * @class port_input_replay
     * @brief Defines an input port that allows reading data from replay files, providing functionality for replaying recorded data in the ADAM system.
     */
    class ADAM_RECREP_API port_input_replay : public port_input
    {
    public:

        static constexpr string_hashed_ct type_name = string_hashed_ct("replay");

        /** @brief Constructs a new input port object. */
        port_input_replay(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input_replay();

        virtual constexpr const string_hashed_ct& get_type_name() const override { return type_name; }
    };
}