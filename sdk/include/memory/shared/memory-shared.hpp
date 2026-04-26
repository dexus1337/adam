#pragma once

/**
 * @file        memory-shared.hpp
 * @author      dexus1337
 * @brief       A singleton class responsible for managing shared memory buffers across processes and modules, providing efficient memory management and interprocess communication.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"
#include "string/string-hashed.hpp"

#include <cstdint>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam 
{
    /**
     * @class   memory_shared
     * @brief   A class responsible for managing shared memory buffers across processes and modules, providing efficient memory management and interprocess communication.
     */
    class ADAM_SDK_API memory_shared 
    {
    public:

        /** @brief Constructs a new memory_shared object. */
        memory_shared(const string_hashed& name);

        /** @brief Destroys the memory_shared object and cleans up resources. */
        ~memory_shared();

        /** @brief Creates the shared memory region, setting up necessary resources. */
        bool create(uint64_t buffer_size);

        /** @brief Opens the shared memory region, setting up necessary resources. */
        bool open();

        /** @brief Shuts down the shared memory region, cleaning up resources. */
        void shutdown();

    protected:

        string_hashed m_name;               /**< The hashed name of the shared memory region, used for identification across processes. */

        void*       m_shared_memory_base;   /**< Base pointer to the shared memory region used for memory buffers. */
        uint64_t    m_shared_memory_size;   /**< Total size of the shared memory region. */

        bool        m_is_owner;             /**< Flag indicating whether this instance is the owner/creator of the shared memory region, responsible for cleanup. */

        #ifdef ADAM_PLATFORM_WINDOWS
        HANDLE      m_shared_memory_handle; /**< Handle to the shared memory object on Windows. */
        #endif
    };
}