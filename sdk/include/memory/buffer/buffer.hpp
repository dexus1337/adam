#pragma once

/**
 * @file    buffer.hpp
 * @author  dexus1337
 * @brief   Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"


#include <atomic>
#include <cstdint>


namespace adam 
{
    /**
     * @class buffer
     * @brief Defines a memory buffer class. Uses shared memory for interprocess availability and efficient memory management.
     */
    class ADAM_SDK_API buffer 
    {
    public:

        /**
         * @brief Constructs a new buffer object.
         */
        buffer();

        /**
         * @brief Destroys the buffer object and cleans up resources.
         */
        ~buffer();

    protected:

        std::atomic<uint32_t> m_ref_count;  /**< Reference count for managing the lifetime of the memory buffer across multiple users. */
        uint64_t m_size;                    /**< The size of the memory buffer in bytes. */
        void* m_data;                       /**< Pointer to the actual memory buffer data. */

    };
}