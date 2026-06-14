#pragma once

/**
 * @file    module-essential.hpp
 * @author  dexus1337
 * @brief   Defines an internal module compiled directly into ADAM to host essential formats, ports, and processors.
 * @version 1.0
 * @date    12.06.2026
 */

#include "module/module.hpp"

namespace adam 
{
    /**
     * @class   module_essential
     * @brief   An internal module compiled directly into ADAM to host essential formats, ports, and processors.
     */
    class ADAM_SDK_API module_essential : public module
    {
    public:
        module_essential();
        ~module_essential();
    };

    extern ADAM_SDK_API module_essential internal_module_essential;
}
