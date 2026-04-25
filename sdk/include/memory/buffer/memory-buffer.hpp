#pragma once

/**
 * @file        memory-buffer.hpp
 * @author      dexus1337
 * @brief       Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class memory_buffer
     * @brief Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
     */
    class ADAM_SDK_API memory_buffer 
    {
    public:

        /**
         * @brief Constructs a new memory_buffer object.
         */
        memory_buffer();

        /**
         * @brief Destroys the memory_buffer object and cleans up resources.
         */
        ~memory_buffer();
    };
}