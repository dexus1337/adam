#pragma once

/**
 * @file    port-internal.hpp
 * @author  dexus1337
 * @brief   Defines a bidirectional internal port for passing data between connections directly
 * @version 1.0
 * @date    17.05.2026
 */

 
#include "api/sdk-api.hpp"
#include "data/port/port-bidirectional.hpp"


namespace adam 
{
    /**
     * @class port_internal
     * @brief Defines a bidirectional internal port for passing data between connections directly
     */
    class ADAM_SDK_API port_internal : public port_bidirectional
    {
    public:

        static constexpr string_hashed_ct type_name = "internal";

        /** @brief Constructs a new internal port object. */
        port_internal(const string_hashed& item_name);

        /** @brief Destroys the internal port object and cleans up resources. */
        ~port_internal();

        virtual const string_hashed_ct& get_type_name() const override { return type_name; }
    };
}