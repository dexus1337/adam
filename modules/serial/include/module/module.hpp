#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class for the Serial (COM/tty) module, providing specific functionality for handling Serial data.
 * @version 1.0
 * @date    28.04.2026
 */


#include "api/api-serial.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::serial
{
    /**
     * @class   module_serial
     * @brief   A class for the Serial (COM/tty) module, providing specific functionality for handling Serial data.
     */
    class ADAM_SERIAL_API module_serial : public module
    {
    public:

        static ADAM_CONSTEXPR uint32_t version = adam::make_version(1, 0, 0);
        
        /**
         * @brief Constructs a new module object.
         */
        module_serial();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_serial();

    };
}

/** @brief Returns a pointer to the Serial module instance. This is the exported function for getting the module. */
extern "C" ADAM_SERIAL_API adam::module* get_adam_module();