#pragma once

/**
 * @file    buffer-manager.hpp
 * @author  dexus1337
 * @brief   A singleton class - 
 *          Global IPC Memory Gateway - 
 *          responsible for managing buffers, allocating and freeing memory, and providing a safe interface for buffer sharing across processes.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <memory>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace adam 
{
    class buffer;
    struct buffer_handle;
    class memory;

    /**
     * @class   buffer_manager
     * @brief   A singleton class for the ADAM controller
     *          Global IPC Memory Gateway - 
     *          responsible for managing buffers, allocating and freeing memory, and providing a safe interface for buffer sharing across processes.
     */
    class ADAM_SDK_API buffer_manager 
    {
    public:

        friend struct buffer_thread_cache;

        static constexpr uint8_t num_capacity_classes   = 30;               /**< Covers up to 4GB buffers safely. */
        static constexpr uint64_t default_chunk_size    = 16 * 1024 * 1024; /**< 16 MB memory chunk allocations. */
        static constexpr size_t batch_transfer_size     = 32;               /**< Buffers to transfer at once. */
        static constexpr size_t max_thread_cache_size   = 64;               /**< Max local buffers before returning a batch. */
        static constexpr uint32_t buffer_free_state     = 0xFFFFFFFF;       /**< Special state indicating a buffer is in the free list. */

        /** @brief Retrieves the singleton instance of the buffer_manager. */
        static buffer_manager& get();

        // Delete copy constructor and assignment operator to prevent copying of the singleton instance
        buffer_manager(const buffer_manager&)               = delete;
        buffer_manager& operator=(const buffer_manager&)    = delete;
        buffer_manager(buffer_manager&&)                    = delete;
        buffer_manager& operator=(buffer_manager&&)         = delete;

        /** @brief Initializes the buffer manager, setting up necessary resources. */
        bool initialize();

        /** @brief Shuts down the buffer manager, cleaning up resources. */
        bool destroy();

        /** @brief Requests a new buffer with the specified size. */
        buffer* request_buffer(uint32_t size);

        /** @brief Returns a buffer back to the manager for cleanup and reuse. */
        void return_buffer(buffer* buffer);

        /** @brief Resolves a buffer handle received over IPC into a usable buffer object. */
        buffer* resolve_handle(const buffer_handle& handle);

    protected:

        static constexpr const char* memory_name_prefix = "adam::buffer_chunk_"; /**< Prefix for shared memory names used for buffer backing. */

        /** @brief Constructs a new buffer_manager object. */
        buffer_manager();

        /** @brief Destroys the buffer_manager object and cleans up resources. */
        ~buffer_manager();

        /** @brief Helper to get the power-of-two size class index. */
        uint8_t get_capacity_class(uint32_t size) const;
        
        /** @brief Helper to allocate a new large memory chunk and split it into buffers for the given pool. */
        bool allocate_pool_block(uint8_t capacity_class);

        /** @brief Moves a batch of buffers from the global pool to a thread-local cache. */
        void request_batch(uint8_t capacity_class, std::vector<buffer*>& local_list);

        /** @brief Moves a batch of buffers from a thread-local cache back to the global pool. */
        void return_batch(uint8_t capacity_class, std::vector<buffer*>& local_list);

        /** @brief Scans existing pool blocks for released buffers and claims them back. */
        bool scavenge_pool_blocks(uint8_t capacity_class);

        /** @brief Safely destroys a surrogate buffer object created by resolve_handle. */
        void destroy_resolved_buffer(buffer* buf);

        struct global_buffer_pool
        {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::vector<buffer*> free_list;
        };

        global_buffer_pool m_pools[num_capacity_classes];                           /**< Global pool indexed by power of 2 size classes. */

        std::mutex m_memory_mutex;                                                  /**< Protects the underlying memory and buffer vectors. */
        std::unordered_map<uint64_t, std::unique_ptr<memory>> m_memory_blocks;      /**< Backing memory segments mapped by unique 64-bit index. */
        std::vector<std::unique_ptr<uint8_t[]>> m_pool_blocks[num_capacity_classes];/**< Contiguous blocks of raw memory for the global pool. */
        std::vector<std::unique_ptr<uint8_t[]>> m_resolved_blocks;                  /**< Chunked allocations for resolved buffers. */
        std::vector<buffer*> m_resolved_free_list;                                  /**< Free list for recycling resolved buffer objects. */

        uint32_t m_block_counter;                                                   /**< Incrementing counter for generating unique shared memory names. */
    };
}