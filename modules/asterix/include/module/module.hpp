#pragma once

/**
 * @file        module.hpp
 * @author      dexus1337
 * @brief       The module class
 * @version     1.0
 * @date        25.04.2026
 */


#include "api/api.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::asterix
{
    /**
     * @class   module_asterix
     * @brief   A class for the ASTERIX module, providing specific functionality for handling ASTERIX data formats.
     */
    class ADAM_ASTERIX_API module_asterix : public module
    {
    public:

        /**
         * @brief Constructs a new module object.
         */
        module_asterix();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_asterix();

    };
}

/** @brief Returns a pointer to the ASTERIX module instance. This is the exported function for getting the module. */
extern "C" const adam::module* ADAM_ASTERIX_API get_adam_module();