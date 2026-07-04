#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class for the Network module, providing specific functionality for handling Network traffic.
 * @version 1.0
 * @date    28.04.2026
 */


#include "api/api-network.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::network
{
    /**
     * @class   module_network
     * @brief   A class for the Network module, providing specific functionality for handling Network traffic.
     */
    class ADAM_NETWORK_API module_network : public module
    {
    public:

        static ADAM_CONSTEXPR uint32_t version = adam::make_version(1, 0, 0);
        
        /**
         * @brief Constructs a new module object.
         */
        module_network();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_network();

    };
}

/** @brief Returns a pointer to the Network module instance. This is the exported function for getting the module. */
extern "C" ADAM_NETWORK_API adam::module* get_adam_module();