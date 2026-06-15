#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/memory.hpp"
#include "types/string-hashed.hpp"
#include "os/os.hpp"
#include "module/internals/essential/module-essential.hpp"

#include <thread>
#include <algorithm>
#include <iostream>
#include <bit>

#ifdef ADAM_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
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
        std::unique_lock<std::shared_mutex> lock(m_memory_mutex);
        
        // Clear the global free lists to prevent dangling pointers
        for (uint8_t i = 0; i < num_capacity_classes; ++i)
        {
            auto& pool = m_pools[i];
            
            {
                spinlock::guard slock(pool.lock);
                pool.free_list.clear();
            }
        }
        
        for (uint8_t i = 0; i < num_capacity_classes; ++i)
            m_pool_blocks[i].clear();
            
        {
            std::lock_guard<std::mutex> resolved_lock(m_resolved_mutex);
            m_resolved_blocks.clear();
            m_resolved_free_list.clear();
        }
        
        for (auto& pair : m_memory_blocks)
        {
            if (pair.second)
                pair.second->destroy();
        }

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
        buf->m_header->ref_count.store(1, std::memory_order_relaxed);
        buf->m_header->size = 0;
        buf->m_header->reference.set_invalid();
        
        return buf;
    }

    void buffer_manager::return_buffer(buffer* buf)
    {
        // If this is a remote buffer, we don't try to reuse it locally.
        // We just delete our local surrogate object and leave the shared ref_count at 0.
        // The owner process will automatically scavenge it!
        if (buf->m_is_resolved)
        {
            destroy_resolved_buffer(buf);
            return;
        }
        
        // Local buffer: Try to transition it back to the free state to recycle immediately.
        uint32_t expected = 0;
        if (buf->m_header->ref_count.compare_exchange_strong(expected, buffer_free_state, std::memory_order_acquire))
        {
            uint8_t capacity_class = get_capacity_class(buf->get_capacity());
            auto& local_list = t_cache.free_lists[capacity_class];

            local_list.push_back(buf);

            if (local_list.size() >= max_thread_cache_size)
                return_batch(capacity_class, local_list); // Hits the spinlock
        }
    }

    buffer* buffer_manager::resolve_handle(const buffer_handle& handle)
    {
        uint64_t map_key = (static_cast<uint64_t>(handle.thread_id) << 32) | static_cast<uint32_t>(handle.memory_index);
        
        memory* target_mem = nullptr;
        
        // 1. Acquire shared lock for reading m_memory_blocks
        {
            std::shared_lock<std::shared_mutex> shared_lock(m_memory_mutex);
            auto it = m_memory_blocks.find(map_key);
            if (it != m_memory_blocks.end())
            {
                target_mem = it->second.get();
            }
        }
        
        // 2. If not found, acquire unique lock and insert
        if (!target_mem)
        {
            std::unique_lock<std::shared_mutex> unique_lock(m_memory_mutex);
            // Double check inside the lock
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
        }
        
        // 3. Acquire resolved free list lock
        buffer* raw_buf = nullptr;
        {
            std::lock_guard<std::mutex> resolved_lock(m_resolved_mutex);
            if (m_resolved_free_list.empty())
            {
                ADAM_CONSTEXPR size_t chunk_size = 1024;
                
                // Allocate raw memory to avoid the constructor call overhead
                auto raw_block = std::unique_ptr<uint8_t[]>(new uint8_t[chunk_size * sizeof(buffer)]);
                buffer* new_block = reinterpret_cast<buffer*>(raw_block.get());
                
                for (size_t i = 0; i < chunk_size; ++i)
                    m_resolved_free_list.push_back(&new_block[i]);
                    
                m_resolved_blocks.push_back(std::move(raw_block));
            }
            
            raw_buf = m_resolved_free_list.back();
            m_resolved_free_list.pop_back();
        }
        
        raw_buf->m_handle       = handle;
        raw_buf->m_header       = reinterpret_cast<buffer::header*>(static_cast<uint8_t*>(target_mem->get()) + handle.offset);
        raw_buf->m_data         = static_cast<uint8_t*>(static_cast<void*>(raw_buf->m_header)) + sizeof(buffer::header);
        raw_buf->m_is_resolved  = true;
        raw_buf->m_data_format  = &data_format_transparent;
        
        return raw_buf;
    }

    void buffer_manager::destroy_resolved_buffer(buffer* buf)
    {
        std::lock_guard<std::mutex> resolved_lock(m_resolved_mutex);
        m_resolved_free_list.push_back(buf);
    }

    buffer_manager::buffer_manager() : m_block_counter(0) 
    {
    }

    buffer_manager::~buffer_manager() 
    {
        destroy();
    }



    bool buffer_manager::allocate_pool_block(uint8_t capacity_class)
    {
        // The actual size this class represents (128, 256, 512, 1024...)
        uint32_t capacity = get_class_capacity(capacity_class); 
        
        // Heavy OS lock (mmap/CreateFileMapping are slow, don't use spinlocks here!)
        std::unique_lock<std::shared_mutex> mem_lock(m_memory_mutex);
        
        os::thread_id tid = os::get_current_thread_id();
        adam::string_hashed mem_name(memory_name_prefix + std::to_string(tid) + "_" + std::to_string(m_block_counter));
        
        // Allocate space for the header region + buffer payload size
        uint64_t actual_chunk_size  = capacity + sizeof(buffer::header);
        uint64_t memory_size        = std::max(static_cast<uint64_t>(default_chunk_size), actual_chunk_size);
        uint64_t chunks             = memory_size / actual_chunk_size;
        
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
            buffer* buf         = &block[i];
            buf->m_handle       = buffer_handle(m_block_counter, static_cast<uint32_t>(i * actual_chunk_size), capacity, tid);
            buf->m_is_resolved  = false;
            buf->m_data_format  = &data_format_transparent;
            buf->m_header       = reinterpret_cast<buffer::header*>(static_cast<uint8_t*>(new_mem->get()) + buf->m_handle.offset);
            
            buf->m_header->ref_count.store(buffer_free_state, std::memory_order_relaxed);
            buf->m_header->capacity         = static_cast<uint32_t>(capacity);
            buf->m_header->size             = 0;
            buf->m_header->start_pos        = 0;
            buf->m_header->data_format_hash = data_format_transparent.get_name().get_hash();
            buf->m_header->timestamp        = 0;
            buf->m_data                     = static_cast<uint8_t*>(static_cast<void*>(buf->m_header)) + sizeof(buffer::header);
            buf->set_referenced_buffer(nullptr);

            new_buffers.push_back(buf);
        }
        
        uint64_t map_key = (static_cast<uint64_t>(tid) << 32) | m_block_counter;
        m_memory_blocks[map_key] = std::move(new_mem);
        m_pool_blocks[capacity_class].push_back(std::move(raw_block));
        
        auto& pool = m_pools[capacity_class];

        // Quickly acquire spinlock, push the thousands of buffers, and release
        {
            spinlock::guard lock(pool.lock);
            pool.free_list.insert(pool.free_list.end(), new_buffers.begin(), new_buffers.end());
        }
        
        m_block_counter++;

        return true;
    }

    bool buffer_manager::scavenge_pool_blocks(uint8_t capacity_class)
    {
        uint64_t capacity           = get_class_capacity(capacity_class); 
        uint64_t actual_chunk_size  = capacity + sizeof(buffer::header);
        uint64_t memory_size        = std::max(static_cast<uint64_t>(default_chunk_size), actual_chunk_size);
        uint64_t chunks             = memory_size / actual_chunk_size;
        
        std::shared_lock<std::shared_mutex> mem_lock(m_memory_mutex);
        
        std::vector<buffer*> recovered;
        
        for (const auto& raw_block : m_pool_blocks[capacity_class])
        {
            buffer* block = reinterpret_cast<buffer*>(raw_block.get());
            for (uint64_t i = 0; i < chunks; ++i)
            {
                buffer* buf = &block[i];
                
                // If ref_count is exactly 0, it means all remote and local processes have released it!
                // Fast relaxed load to avoid dirtying the cache line if the buffer is still in use
                if (buf->m_header->ref_count.load(std::memory_order_relaxed) != 0)
                    continue;

                uint32_t expected = 0;
                if (buf->m_header->ref_count.compare_exchange_strong(expected, buffer_free_state, std::memory_order_acquire))
                    recovered.push_back(buf);
            }
        }
        
        if (recovered.empty())
            return false;
            
        auto& pool = m_pools[capacity_class];

        {
            spinlock::guard lock(pool.lock);
            pool.free_list.insert(pool.free_list.end(), recovered.begin(), recovered.end());
        }
        
        return true;
    }

    void buffer_manager::request_batch(uint8_t capacity_class, std::vector<buffer*>& local_list)
    {
        auto& pool = m_pools[capacity_class];

        while (true)
        {
            {
                spinlock::guard lock(pool.lock);
                if (!pool.free_list.empty())
                {
                    size_t count = std::min(static_cast<size_t>(batch_transfer_size), pool.free_list.size());
                    auto start_it = pool.free_list.end() - count;
                    local_list.insert(local_list.end(), start_it, pool.free_list.end());
                    pool.free_list.erase(start_it, pool.free_list.end());
                    return;
                }
            }
            
            // Before asking the OS for a new 16MB chunk, quickly scan existing chunks 
            // for buffers that remote processes have released!
            if (!scavenge_pool_blocks(capacity_class))
            {
                if (!allocate_pool_block(capacity_class))
                    return; 
            }
        }
    }

    void buffer_manager::return_batch(uint8_t capacity_class, std::vector<buffer*>& local_list)
    {
        if (local_list.empty()) return;

        auto& pool = m_pools[capacity_class];

        {
            spinlock::guard lock(pool.lock);
            size_t count = std::min(static_cast<size_t>(batch_transfer_size), local_list.size());
            auto start_it = local_list.end() - count;
            pool.free_list.insert(pool.free_list.end(), start_it, local_list.end());
            local_list.erase(start_it, local_list.end());
        }
    }
}