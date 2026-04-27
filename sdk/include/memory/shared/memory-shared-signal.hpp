#pragma once

/**
 * @file        memory_shared-signal.hpp
 * @author      dexus1337
 * @brief       A class to allow signaling for shared memory communication, providing a mechanism for interprocess synchronization and event notification between the main memory_shared 
 *              and external processes.
 * @version     1.0
 * @date        27.04.2026
 */

 
#include "api/api.hpp"

#ifdef   ADAM_PLATFORM_LINUX
#include <semaphore.h>
#endif
#include <chrono>


namespace adam 
{
    class memory_shared;

    /**
     * @class   memory_shared_signal
     * @brief   A class to allow signaling for shared memory communication, providing a mechanism for interprocess synchronization and event notification between the main memory_shared 
     *          and external processes.
     */
    class ADAM_SDK_API memory_shared_signal
    {
    public:

        /** @brief Constructs a new memory_shared_signal object.*/
        memory_shared_signal(memory_shared* shared_memory);

        /** @brief Destroys the memory_shared_signal object and cleans up resources.*/
        ~memory_shared_signal();

        /** @brief Create the memory_shared_signal backend, OS-specific initialization.*/
        bool create();

        /** @brief Open existing backend.*/
        bool open();

        /** @brief Releases the signal and stops any queued waits */
        bool destroy();

        /** @brief Send the signal activation. */
        bool notify();

        /** @brief Waits for the signal activation. */
        bool wait(int32_t timeout_ms = -1);

        /** @brief Wait for the signal activation. Compatible with std::chrono::duration */
        template <typename rep, typename period>
        bool wait(std::chrono::duration<rep, period> timeout = std::chrono::duration<rep, period>::max()) { return wait(timeout == std::chrono::duration<rep, period>::max() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()); }

    protected:

        memory_shared* m_shared_memory;   /**< Shared memory instance used identification of the signal semaphore. */

        #ifdef   ADAM_PLATFORM_LINUX
        sem_t* m_sem;
        #elifdef ADAM_PLATFORM_WINDOWS
        HANDLE m_handle;
        #endif

    };
}