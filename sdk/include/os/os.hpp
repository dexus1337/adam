#pragma once

/**
 * @file    os.hpp
 * @author  dexus1337
 * @brief   Operation System functions
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"
#include <cstdint>


namespace adam::os
{
    using thread_id = uint64_t;

    extern ADAM_SDK_API thread_id get_current_thread_id();
}