#pragma once

/**
 * @file    port-input-replay.hpp
 * @author  dexus1337
 * @brief   Defines an input port that allows reading data from replay files, providing functionality for replaying recorded data in the ADAM system.
 * @version 1.0
 * @date    11.05.2026
 */


#include "api/api-recrep.hpp"
 
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

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "replay"_ct; }

        static const configuration_parameter_list& get_default_parameters();

        /** @brief Constructs a new input port object. */
        port_input_replay(const string_hashed& item_name);

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input_replay();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }; /**< THANK YOU MSCV for your really good constexp eval. Thats why we have to do such beautiful things */

        /** @brief Checks for available file(s) */
        virtual bool start() override;

        /** @brief Releases resources */
        virtual bool stop() override;

        /** @brief Protoype function for data input */
        virtual bool read(buffer*& buff) override;
    };
}