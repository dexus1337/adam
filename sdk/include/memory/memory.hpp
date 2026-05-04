#pragma once

/**
 * @file    memory.hpp
 * @author  dexus1337
 * @brief   A singleton class responsible for managing shared memory across processes and modules, 
 *          providing efficient memory management and interprocess communication.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "string/string-hashed.hpp"

#include <cstdint>

#ifdef   ADAM_PLATFORM_WINDOWS
#include <windows.h>
#elifdef ADAM_PLATFORM_LINUX
#include <semaphore>
#endif

namespace adam 
{
    /**
     * @class   memory
     * @brief   A class responsible for managing shared memory across processes and modules, 
     *          providing efficient memory management and interprocess communication.
     */
    class ADAM_SDK_API memory 
    {
    public:

        /** @brief Constructs a new memory object. */
        memory(const string_hashed& name);

        /** @brief Destroys the memory object and cleans up resources. */
        ~memory();

        bool is_active()                            const { return m_b_active; }
        bool is_owner()                             const { return m_is_owner; }
        void* get()                                 const { return reinterpret_cast<uint8_t*>(m_shared_memory_base) + m_memory_offset; }
        uint64_t get_size()                         const { return m_shared_memory_size > m_memory_offset ? m_shared_memory_size - m_memory_offset : 0; }
        const string_hashed& get_name()             const { return m_name; }

        /** @brief Creates the shared memory region, setting up necessary resources. */
        virtual bool create(uint64_t buffer_size);

        /** @brief Opens the shared memory region, setting up necessary resources. */
        virtual bool open();

        /** @brief Unsets the active flag. */
        void disable() { m_b_active = false; }

        /** @brief Destroys down the shared memory region, cleaning up resources. */
        virtual bool destroy();

    protected:

        string_hashed m_name;               /**< The hashed name of the shared memory region, used for identification across processes. */

        bool        m_b_active;             /**< Flag indicating whether the shared memory region is currently active. Can be used for threads as loop condition. */
        bool        m_is_owner;             /**< Flag indicating whether this instance is the owner/creator of the shared memory region, responsible for cleanup. */

        void*       m_shared_memory_base;   /**< Base pointer to the shared memory region used for memory buffers. */
        uint64_t    m_shared_memory_size;   /**< Total size of the shared memory region. */
        uint32_t    m_memory_offset;        /**< Offset to the start of usable shared memory (used for signal placement). */

        #ifdef ADAM_PLATFORM_WINDOWS
        HANDLE      m_shared_memory_handle; /**< Handle to the shared memory object on Windows. */
        #endif
    };
}