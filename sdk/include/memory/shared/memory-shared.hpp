#pragma once

/**
 * @file    memory-shared.hpp
 * @author  dexus1337
 * @brief   A singleton class responsible for managing shared memory buffers across processes and modules, 
 *              providing efficient memory management and interprocess communication.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "string/string-hashed.hpp"
#include "memory-shared-signaled.hpp"

#include <cstdint>

#ifdef   ADAM_PLATFORM_WINDOWS
#include <windows.h>
#elifdef ADAM_PLATFORM_LINUX
#include <semaphore>
#endif

namespace adam 
{
    /**
     * @class   memory_shared
     * @brief   A class responsible for managing shared memory buffers across processes and modules, 
     *          providing efficient memory management and interprocess communication.
     */
    class ADAM_SDK_API memory_shared 
    {
    public:

        /** @brief Constructs a new memory_shared object. */
        memory_shared(const string_hashed& name);

        /** @brief Destroys the memory_shared object and cleans up resources. */
        ~memory_shared();

        /** @brief Retrieves the base pointer to the shared memory region. */
        void* get()                                 const { return m_shared_memory_base; }

        /** @brief Retrieves the size of the shared memory region. */
        uint64_t get_size()                         const { return m_shared_memory_size; }

        /** @brief Retrieves the hashed name of the shared memory region. */
        const string_hashed& get_name()             const { return m_name; }

        #ifdef ADAM_PLATFORM_LINUX
        /** @brief Retrieves the semaphore used for signaling on Linux. */
        sem_t* get_signal_semaphore()               const { return m_signal_sem; }
        #endif

        memory_shared_signaled& signal() { return m_signal; }

        /** @brief Creates the shared memory region, setting up necessary resources. */
        bool create(uint64_t buffer_size);

        /** @brief Opens the shared memory region, setting up necessary resources. */
        bool open();

        /** @brief Destroys down the shared memory region, cleaning up resources. */
        bool destroy();

    protected:

        string_hashed m_name;               /**< The hashed name of the shared memory region, used for identification across processes. */

        bool        m_is_owner;             /**< Flag indicating whether this instance is the owner/creator of the shared memory region, responsible for cleanup. */

        void*       m_shared_memory_base;   /**< Base pointer to the shared memory region used for memory buffers. */
        uint64_t    m_shared_memory_size;   /**< Total size of the shared memory region. */

        #ifdef   ADAM_PLATFORM_LINUX
        sem_t*      m_signal_sem;           /**< On linux, we will use a memory semaphore for signaling, as its way faster than named semaphores */
        #elifdef ADAM_PLATFORM_WINDOWS
        HANDLE      m_shared_memory_handle; /**< Handle to the shared memory object on Windows. */
        #endif

        memory_shared_signaled m_signal;      /**< Signal object for interprocess synchronization and event notification. */
    };
}