#pragma once

/**
 * @file    buffer-manager.hpp
 * @author  dexus1337
 * @brief   A singleton class responsible for managing memory buffers
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include <cstdint>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam 
{
    class buffer;

    /**
     * @class   buffer_manager
     * @brief   A singleton class responsible for managing buffers
     */
    class ADAM_SDK_API buffer_manager 
    {
    public:

        /** @brief Retrieves the singleton instance of the buffer_manager. */
        static buffer_manager& get();

        // Delete copy constructor and assignment operator to prevent copying of the singleton instance
        buffer_manager(const buffer_manager&) = delete;
        void operator=(const buffer_manager&) = delete;

        /** @brief Initializes the buffer manager, setting up necessary resources. */
        bool initialize();

        /** @brief Shuts down the buffer manager, cleaning up resources. */
        bool destroy();

        /** @brief Requests a new buffer with the specified size. */
        buffer* request_buffer(uint64_t size);

        /** @brief Returns a buffer back to the manager for cleanup and reuse. */
        void return_buffer(buffer* buffer);

    protected:

        /** @brief Constructs a new buffer_manager object. */
        buffer_manager();

        /** @brief Destroys the buffer_manager object and cleans up resources. */
        ~buffer_manager();
    };
}