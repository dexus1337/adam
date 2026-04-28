#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   The module class for the Replay/Recording module, providing specific functionality for handling replay/recording data formats.
 * @version 1.0
 * @date    25.04.2026
 */


#include "api/api.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::recrep
{
    /**
     * @class   module_recrep
     * @brief   A class for the Replay/Recording module, providing specific functionality for handling replay/recording data formats.
     */
    class ADAM_RECREP_API module_recrep : public module
    {
    public:

        static constexpr uint32_t version = adam::make_version(1, 0, 0);
        
        /**
         * @brief Constructs a new module object.
         */
        module_recrep();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module_recrep();

    };
}

/** @brief Returns a pointer to the Replay/Recording module instance. This is the exported function for getting the module. */
extern "C" adam::module* ADAM_RECREP_API get_adam_module();