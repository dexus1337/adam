#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/memory.hpp"
#include "types/string-hashed.hpp"
#include "os/os.hpp"

#include <thread>
#include <algorithm>
#include <iostream>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef ADAM_CPU_X64
#include <immintrin.h>
#endif

namespace adam 
{
    /** @brief Defines the thread-local cache structure. */
    struct buffer_thread_cache
    {
        std::vector<buffer*> free_lists[buffer_manager::num_capacity_classes];
            
        ~buffer_thread_cache();
    };

    // Define the unique thread-local cache for every thread that enters the system
    static thread_local buffer_thread_cache t_cache;

    buffer_manager& buffer_manager::get()
    {
        static buffer_manager instance;
        return instance;
    }

    buffer_thread_cache::~buffer_thread_cache()
    {
        auto& manager = buffer_manager::get();
        
        // When a thread dies, dump all its hoarded buffers back into the global pool
        for (uint8_t i = 0; i < buffer_manager::num_capacity_classes; ++i)
        {
            while (!free_lists[i].empty())
            {
                manager.return_batch(i, free_lists[i]);
            }
        }
    }

    bool buffer_manager::initialize()
    {
        return true;
    }

    bool buffer_manager::destroy()
    {
        std::lock_guard<std::mutex> lock(m_memory_mutex);
        
        // Clear the global free lists to prevent dangling pointers
        for (uint8_t i = 0; i < num_capacity_classes; ++i)
        {
            auto& pool = m_pools[i];
            
            while (pool.lock.test_and_set(std::memory_order_acquire))
            {
                #ifdef ADAM_CPU_X64
                _mm_pause();
                #endif 
            }
            pool.free_list.clear();
            pool.lock.clear(std::memory_order_release);
        }
        
        for (uint8_t i = 0; i < num_capacity_classes; ++i)
            m_pool_blocks[i].clear();
            
        m_resolved_blocks.clear();
        m_resolved_free_list.clear();
        
        m_memory_blocks.clear();
        
        // Clear the thread-local cache for the current thread to prevent returning 
        // dangling pointers in sequentially executed tests or on thread exit.
        for (uint8_t i = 0; i < num_capacity_classes; ++i)
            t_cache.free_lists[i].clear();

        return true;
    }

    buffer* buffer_manager::request_buffer(uint32_t capacity)
    {
        uint8_t capacity_class = get_capacity_class(capacity);
        auto& local_list = t_cache.free_lists[capacity_class];

        // The holy grail of lock-free allocators: 99% of the time, this `if` is false!
        if (local_list.empty())
            request_batch(capacity_class, local_list); // Hits the spinlock

        if (local_list.empty())
            return nullptr; // Out of memory

        buffer* buf = local_list.back();
        local_list.pop_back();
        buf->m_ref_count->store(1, std::memory_order_relaxed);
        
        return buf;
    }

    void buffer_manager::return_buffer(buffer* buffer)
    {
        // If this is a remote buffer, we don't try to reuse it locally.
        // We just delete our local surrogate object and leave the shared ref_count at 0.
        // The owner process will automatically scavenge it!
        if (buffer->m_is_resolved)
        {
            destroy_resolved_buffer(buffer);
            return;
        }
        
        // Local buffer: Try to transition it back to the free state to recycle immediately.
        uint32_t expected = 0;
        if (buffer->m_ref_count->compare_exchange_strong(expected, buffer_free_state, std::memory_order_acquire))
        {
            uint8_t capacity_class = get_capacity_class(buffer->get_capacity());
            auto& local_list = t_cache.free_lists[capacity_class];

            local_list.push_back(buffer);

            if (local_list.size() >= max_thread_cache_size)
                return_batch(capacity_class, local_list); // Hits the spinlock
        }
    }

    buffer* buffer_manager::resolve_handle(const buffer_handle& handle)
    {
        std::lock_guard<std::mutex> mem_lock(m_memory_mutex);
        
        uint64_t map_key = (static_cast<uint64_t>(handle.thread_id) << 32) | static_cast<uint32_t>(handle.memory_index);
        
        memory* target_mem = nullptr;
        auto it = m_memory_blocks.find(map_key);
        
        if (it != m_memory_blocks.end())
        {
            target_mem = it->second.get();
        }
        else
        {
            adam::string_hashed mem_name(memory_name_prefix + std::to_string(handle.thread_id) + "_" + std::to_string(handle.memory_index));
            auto new_mem = std::make_unique<memory>(mem_name);
            
            if (!new_mem->open())
                return nullptr;
                
            target_mem = new_mem.get();
            m_memory_blocks[map_key] = std::move(new_mem);
        }
        
        if (m_resolved_free_list.empty())
        {
            constexpr size_t chunk_size = 1024;
            
            // Allocate raw memory to avoid the constructor call overhead
            auto raw_block = std::unique_ptr<uint8_t[]>(new uint8_t[chunk_size * sizeof(buffer)]);
            buffer* new_block = reinterpret_cast<buffer*>(raw_block.get());
            
            for (size_t i = 0; i < chunk_size; ++i)
                m_resolved_free_list.push_back(&new_block[i]);
                
            m_resolved_blocks.push_back(std::move(raw_block));
        }
        
        buffer* raw_buf = m_resolved_free_list.back();
        m_resolved_free_list.pop_back();
        
        raw_buf->m_memory_index = handle.memory_index;
        raw_buf->m_thread_id = handle.thread_id;
        raw_buf->m_offset = handle.offset;
        raw_buf->m_capacity = handle.size;
        raw_buf->m_ref_count = reinterpret_cast<std::atomic<uint32_t>*>(static_cast<uint8_t*>(target_mem->get()) + handle.offset);
        raw_buf->m_data = static_cast<uint8_t*>(target_mem->get()) + handle.offset + 8;
        raw_buf->m_is_resolved = true;
        raw_buf->m_data_format = &data_format_transparent;
        
        return raw_buf;
    }

    void buffer_manager::destroy_resolved_buffer(buffer* buf)
    {
        std::lock_guard<std::mutex> mem_lock(m_memory_mutex);
        m_resolved_free_list.push_back(buf);
    }

    buffer_manager::buffer_manager() : m_block_counter(0) 
    {
    }

    buffer_manager::~buffer_manager() 
    {
        destroy();
    }

    uint8_t buffer_manager::get_capacity_class(uint32_t size) const
    {
        if (size <= 128) 
            return 0;
        
        uint8_t capacity_class = 0;
        uint64_t current_size = 128; // 64-bit to prevent overflow when checking bounds near 4GB
        
        // Find the lowest power of 2 that completely fits the requested size
        while (current_size < size && capacity_class < num_capacity_classes - 1)
        {
            current_size <<= 1;
            capacity_class++;
        }
        return capacity_class;
    }

    bool buffer_manager::allocate_pool_block(uint8_t capacity_class)
    {
        // The actual size this class represents (128, 256, 512, 1024...)
        uint64_t buffer_size = 1ULL << (capacity_class + 7); 
        
        // Heavy OS lock (mmap/CreateFileMapping are slow, don't use spinlocks here!)
        std::lock_guard<std::mutex> mem_lock(m_memory_mutex);
        
        os::thread_id tid = os::get_current_thread_id();
        adam::string_hashed mem_name(memory_name_prefix + std::to_string(tid) + "_" + std::to_string(m_block_counter));
        
        // 8 bytes padding per buffer to store the shared reference count
        uint64_t actual_chunk_size = buffer_size + 8;
        uint64_t memory_size = std::max(static_cast<uint64_t>(default_chunk_size), actual_chunk_size);
        uint64_t chunks = memory_size / actual_chunk_size;
        
        auto new_mem = std::make_unique<memory>(mem_name);
        if (!new_mem->create(memory_size))
            return false;
        
        // Allocate raw memory to avoid the 1-million-iteration constructor call overhead entirely
        auto raw_block = std::unique_ptr<uint8_t[]>(new uint8_t[chunks * sizeof(buffer)]);
        buffer* block = reinterpret_cast<buffer*>(raw_block.get());

        std::vector<buffer*> new_buffers;
        new_buffers.reserve(chunks);
        
        for (uint64_t i = 0; i < chunks; ++i)
        {
            buffer* buf = &block[i];
            buf->m_capacity = static_cast<uint32_t>(buffer_size);
            buf->m_offset = static_cast<uint32_t>(i * actual_chunk_size);
            buf->m_memory_index = m_block_counter;
            buf->m_thread_id = tid;
            buf->m_is_resolved = false;
            buf->m_data_format = &data_format_transparent;
            
            buf->m_ref_count = reinterpret_cast<std::atomic<uint32_t>*>(static_cast<uint8_t*>(new_mem->get()) + buf->m_offset);
            new (buf->m_ref_count) std::atomic<uint32_t>(buffer_free_state); // Initialize cross-process atomic
            
            buf->m_data = static_cast<uint8_t*>(new_mem->get()) + buf->m_offset + 8;
            
            new_buffers.push_back(buf);
        }
        
        uint64_t map_key = (static_cast<uint64_t>(tid) << 32) | m_block_counter;
        m_memory_blocks[map_key] = std::move(new_mem);
        m_pool_blocks[capacity_class].push_back(std::move(raw_block));
        
        // Quickly acquire spinlock, push the thousands of buffers, and release
        auto& pool = m_pools[capacity_class];

        while (pool.lock.test_and_set(std::memory_order_acquire))
        {
            #ifdef ADAM_CPU_X64
            _mm_pause();
            #endif 
        }

        pool.free_list.insert(pool.free_list.end(), new_buffers.begin(), new_buffers.end());
        pool.lock.clear(std::memory_order_release);
        
        m_block_counter++;

        return true;
    }

    bool buffer_manager::scavenge_pool_blocks(uint8_t capacity_class)
    {
        uint64_t buffer_size = 1ULL << (capacity_class + 7); 
        uint64_t actual_chunk_size = buffer_size + 8;
        uint64_t memory_size = std::max(static_cast<uint64_t>(default_chunk_size), actual_chunk_size);
        uint64_t chunks = memory_size / actual_chunk_size;
        
        std::lock_guard<std::mutex> mem_lock(m_memory_mutex);
        
        std::vector<buffer*> recovered;
        
        for (const auto& raw_block : m_pool_blocks[capacity_class])
        {
            buffer* block = reinterpret_cast<buffer*>(raw_block.get());
            for (uint64_t i = 0; i < chunks; ++i)
            {
                buffer* buf = &block[i];
                uint32_t expected = 0;
                
                // If ref_count is exactly 0, it means all remote and local processes have released it!
                if (buf->m_ref_count->compare_exchange_strong(expected, buffer_free_state, std::memory_order_acquire))
                {
                    recovered.push_back(buf);
                }
            }
        }
        
        if (recovered.empty())
            return false;
            
        auto& pool = m_pools[capacity_class];

        while (pool.lock.test_and_set(std::memory_order_acquire)) 
        { 
            #ifdef ADAM_CPU_X64 
            _mm_pause(); 
            #endif 
        }

        pool.free_list.insert(pool.free_list.end(), recovered.begin(), recovered.end());
        pool.lock.clear(std::memory_order_release);
        
        return true;
    }

    void buffer_manager::request_batch(uint8_t capacity_class, std::vector<buffer*>& local_list)
    {
        auto& pool = m_pools[capacity_class];

        // Acquire global spinlock
        while (pool.lock.test_and_set(std::memory_order_acquire)) 
        {
            #ifdef ADAM_CPU_X64
            _mm_pause();
            #endif 
        }

        if (pool.free_list.empty())
        {
            pool.lock.clear(std::memory_order_release);
            
            // Before asking the OS for a new 16MB chunk, quickly scan existing chunks 
            // for buffers that remote processes have released!
            if (!scavenge_pool_blocks(capacity_class))
            {
                if (!allocate_pool_block(capacity_class))
                    return; 
            }
            
            // Re-acquire spinlock now that memory is ready
            while (pool.lock.test_and_set(std::memory_order_acquire)) 
            {
                #ifdef ADAM_CPU_X64
                _mm_pause();
                #endif 
            }
        }

        // Steal a batch of pointers for the local thread
        size_t count = std::min(batch_transfer_size, pool.free_list.size());
        for (size_t i = 0; i < count; ++i)
        {
            local_list.push_back(pool.free_list.back());
            pool.free_list.pop_back();
        }

        // Release global spinlock
        pool.lock.clear(std::memory_order_release);
    }

    void buffer_manager::return_batch(uint8_t capacity_class, std::vector<buffer*>& local_list)
    {
        auto& pool = m_pools[capacity_class];

        // Acquire global spinlock
        while (pool.lock.test_and_set(std::memory_order_acquire)) 
        { 
            #ifdef ADAM_CPU_X64
            _mm_pause(); 
            #endif 
        }

        // Dump excess local buffers back into the global pool
        size_t count = std::min(batch_transfer_size, local_list.size());
        for (size_t i = 0; i < count; ++i)
        {
            pool.free_list.push_back(local_list.back());
            local_list.pop_back();
        }

        // Release global spinlock
        pool.lock.clear(std::memory_order_release);
    }
}