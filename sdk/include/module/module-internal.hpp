#pragma once

/**
 * @file    module-internal.hpp
 * @author  dexus1337
 * @brief   Defines an internal module compiled directly into ADAM to host internal formats, ports, and processors.
 * @version 1.0
 * @date    12.06.2026
 */

#include "module/module.hpp"

namespace adam 
{
    /**
     * @class   module_internal
     * @brief   An internal module compiled directly into ADAM to host internal formats, ports, and processors.
     */
    class ADAM_SDK_API module_internal : public module
    {
    public:
        module_internal();
        ~module_internal();
    };
}
