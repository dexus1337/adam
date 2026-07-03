#pragma once

/**
 * @file    memory.hpp
 * @author  dexus1337
 * @brief   A singleton class responsible for managing shared memory across processes and modules, 
 *          providing efficient memory management and interprocess communication.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include <cstdint>

#ifdef   ADAM_PLATFORM_WINDOWS
#include <windows.h>
#elifdef ADAM_PLATFORM_LINUX
#include <semaphore>
#endif

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "types/enum-bit-operations.hpp"


namespace adam 
{
        enum state_flags : uint8_t
        {
            state_zero      = 0,
            state_created   = 1 << 0,
            state_active    = 1 << 1,
            state_owned     = 1 << 2,
        };
        enable_enum_bit_operations(state_flags);

    /**
     * @class   memory
     * @brief   A class responsible for managing shared memory across processes and modules, 
     *          providing efficient memory management and interprocess communication.
     */
    class ADAM_SDK_API memory 
    {
    public:

        /** @brief Constructs a new memory object. */
        memory(const string_hashed& name = string_hashed());

        /** @brief Destroys the memory object and cleans up resources. */
        ~memory();

        inline bool is_created()                const { return m_s_state & state_created; }
        inline bool is_active()                 const { return m_s_state & state_active; }
        inline bool is_owner()                  const { return m_s_state & state_owned; }
        inline void* get()                      const { return reinterpret_cast<uint8_t*>(m_shared_memory_base) + m_memory_offset; }
        inline uint64_t get_size()              const { return m_shared_memory_size > m_memory_offset ? m_shared_memory_size - m_memory_offset : 0; }
        inline const string_hashed& get_name()  const { return m_name; }


        inline void set_name(const string_hashed& new_name) { m_name = new_name; }
        inline void disable()                               { m_s_state &= ~state_active; }

        /** @brief Creates the shared memory region, setting up necessary resources. */
        virtual bool create(uint64_t buffer_size);

        /** @brief Opens the shared memory region, setting up necessary resources. */
        virtual bool open();

        /** @brief Destroys down the shared memory region, cleaning up resources. */
        virtual bool destroy();

    protected:

        string_hashed   m_name;                 /**< The hashed name of the shared memory region, used for identification across processes. */

        state_flags     m_s_state;              /**< Several Flags that can be used by users/thread. */

        void*           m_shared_memory_base;   /**< Base pointer to the shared memory region used for memory buffers. */
        uint64_t        m_shared_memory_size;   /**< Total size of the shared memory region. */
        uint32_t        m_memory_offset;        /**< Offset to the start of usable shared memory (used for signal placement). */

        #ifdef ADAM_PLATFORM_WINDOWS
        HANDLE          m_shared_memory_handle; /**< Handle to the shared memory object on Windows. */
        #endif
    };
}