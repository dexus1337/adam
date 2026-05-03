#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class for the UDP Network module, providing specific functionality for handling UDP network traffic.
 * @version 1.0
 * @date    28.04.2026
 */


#include "api/api.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::udp
{
    /**
     * @class   module_udp
     * @brief   A class for the UDP Network module, providing specific functionality for handling UDP network traffic.
     */
    class ADAM_UDP_API module_udp : public module
    {
    public:

        static constexpr uint32_t version = adam::make_version(1, 0, 0);
        
        /**
         * @brief Constructs a new module object.
         */
        module_udp();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_udp();

    };
}

/** @brief Returns a pointer to the UDP Network module instance. This is the exported function for getting the module. */
extern "C" ADAM_UDP_API adam::module* get_adam_module();