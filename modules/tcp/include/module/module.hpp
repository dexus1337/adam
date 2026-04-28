#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class for the TCP network module, providing specific functionality for handling TCP network traffic.
 * @version 1.0
 * @date    28.04.2026
 */


#include "api/api.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::tcp
{
    /**
     * @class   module_tcp
     * @brief   A class for the TCP network module, providing specific functionality for handling TCP network traffic.
     */
    class ADAM_TCP_API module_tcp : public module
    {
    public:

        static constexpr uint32_t version = adam::make_version(1, 0, 0);
        
        /**
         * @brief Constructs a new module object.
         */
        module_tcp();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_tcp();

    };
}

/** @brief Returns a pointer to the TCP network module instance. This is the exported function for getting the module. */
extern "C" adam::module* ADAM_TCP_API get_adam_module();