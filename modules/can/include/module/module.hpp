#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class
 * @version 1.0
 * @date    25.04.2026
 */


#include "api/api.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::can
{
    /**
     * @class   module_can
     * @brief   A class for the CAN module, providing specific functionality for handling CAN data formats.
     */
    class ADAM_CAN_API module_can : public module
    {
    public:

        static constexpr uint32_t version = adam::make_version(1, 0, 0);

        /**
         * @brief Constructs a new module object.
         */
        module_can();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_can();

    };
}

/** @brief Returns a pointer to the CAN module instance. This is the exported function for getting the module. */
extern "C" ADAM_CAN_API adam::module* get_adam_module();