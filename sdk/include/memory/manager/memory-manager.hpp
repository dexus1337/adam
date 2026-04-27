#pragma once

/**
 * @file        memory-manager.hpp
 * @author      dexus1337
 * @brief       A singleton class responsible for managing memory buffers
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include <cstdint>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam 
{
    class memory_buffer;

    /**
     * @class   memory_manager
     * @brief   A singleton class responsible for managing memory buffers
     */
    class ADAM_SDK_API memory_manager 
    {
    public:

        static constexpr uint32_t default_buffer_size = 0x1000; /**< Default size for memory buffers, 4KB */

        /** @brief Retrieves the singleton instance of the memory_manager. */
        static memory_manager& get();

        // Delete copy constructor and assignment operator to prevent copying of the singleton instance
        memory_manager(const memory_manager&) = delete;
        void operator=(const memory_manager&) = delete;

        /** @brief Initializes the memory manager, setting up necessary resources. */
        bool initialize(uint32_t buffer_size = default_buffer_size); /**< Initializes the memory manager, setting up necessary resources. */

        /** @brief Shuts down the memory manager, cleaning up resources. */
        void destroy();

        /** @brief Requests a new memory buffer with the specified size. */
        memory_buffer* request_buffer(uint64_t size);

        /** @brief Returns a memory buffer back to the manager for cleanup and reuse. */
        void return_buffer(memory_buffer* buffer);

    protected:

        /** @brief Constructs a new memory_manager object. */
        memory_manager();

        /** @brief Destroys the memory_manager object and cleans up resources. */
        ~memory_manager();

        void*       m_shared_memory_base;   /**< Base pointer to the shared memory region used for memory buffers. */
        uint64_t    m_shared_memory_size;   /**< Total size of the shared memory region. */

        #ifdef ADAM_PLATFORM_WINDOWS
        HANDLE      m_shared_memory_handle; /**< Handle to the shared memory object on Windows. */
        #endif
    };
}