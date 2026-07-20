#include <gtest/gtest.h>
#include <memory/buffer/buffer-manager.hpp>
#include <memory/buffer/buffer.hpp>

#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <cstdlib>

class buffer_manager_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests that the singleton can initialize and destroy multiple times cleanly. */
TEST_F(buffer_manager_test, lifecycle)
{
    EXPECT_TRUE(adam::buffer_manager::get().destroy());
    EXPECT_TRUE(adam::buffer_manager::get().initialize());
}

/** @brief Tests requesting and returning a buffer uses the lock-free thread-local caches properly. */
TEST_F(buffer_manager_test, request_and_return)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* buf1 = mgr.request_buffer(128);
    ASSERT_NE(buf1, nullptr);
    
    // Return it to local cache
    buf1->release();

    // Since it is now in the LIFO lock-free stack, asking for the same size should yield the exact same pointer
    adam::buffer* buf2 = mgr.request_buffer(128);
    EXPECT_EQ(buf1, buf2);

    buf2->release();
}

/** @brief Tests allocating buffers of wildly different sizes correctly hits different capacity classes. */
TEST_F(buffer_manager_test, capacity_classes)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* buf_small = mgr.request_buffer(16);
    adam::buffer* buf_med   = mgr.request_buffer(1024);
    adam::buffer* buf_large = mgr.request_buffer(1024 * 1024); // 1 MB

    ASSERT_NE(buf_small, nullptr);
    ASSERT_NE(buf_med, nullptr);
    ASSERT_NE(buf_large, nullptr);

    EXPECT_GE(buf_small->get_capacity(), 16u);
    EXPECT_GE(buf_med->get_capacity(), 1024u);
    EXPECT_GE(buf_large->get_capacity(), 1024u * 1024u);

    buf_small->release();
    buf_med->release();
    buf_large->release();
}

/** @brief Tests allocating a buffer larger than the default 16MB chunk size. */
TEST_F(buffer_manager_test, oversized_buffer)
{
    auto& mgr = adam::buffer_manager::get();
    
    // Request a buffer that is larger than the default_chunk_size (16MB)
    const uint32_t oversized_size = 32 * 1024 * 1024; // 32 MB
    adam::buffer* buf = mgr.request_buffer(oversized_size);

    ASSERT_NE(buf, nullptr);
    EXPECT_GE(buf->get_capacity(), oversized_size);

    // Verify we can write to the beginning and end of the memory safely
    uint8_t* data = static_cast<uint8_t*>(buf->data());
    data[0] = 0xAA;
    data[oversized_size - 1] = 0xBB;
    
    EXPECT_EQ(data[0], 0xAA);
    EXPECT_EQ(data[oversized_size - 1], 0xBB);

    buf->release();
}

/** @brief Tests resolving a buffer handle back into a buffer object. */
TEST_F(buffer_manager_test, resolve_handle)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* buf = mgr.request_buffer(256);
    ASSERT_NE(buf, nullptr);
    
    buf->set_size(100);
    buf->set_timestamp(42);
    
    const char* payload = "Test data for handle resolution";
    buf->fill_data(payload, static_cast<uint32_t>(strlen(payload) + 1));
    
    adam::buffer_handle handle = buf->get_handle();
    EXPECT_TRUE(handle.is_valid());
    
    // Resolve the handle
    adam::buffer* resolved = mgr.resolve_handle(handle);
    ASSERT_NE(resolved, nullptr);
    
    EXPECT_EQ(resolved->get_capacity(), buf->get_capacity());
    EXPECT_EQ(resolved->get_size(), buf->get_size());
    EXPECT_EQ(resolved->get_timestamp(), buf->get_timestamp());
    EXPECT_STREQ(resolved->get_data_as<char>(), payload);
    
    // Release both (resolved buffer release will recycle the surrogate)
    resolved->release();
    buf->release();
}

/** @brief Tests that multiple threads can concurrently resolve buffer handles safely. */
TEST_F(buffer_manager_test, concurrent_resolve)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* buf = mgr.request_buffer(256);
    ASSERT_NE(buf, nullptr);
    adam::buffer_handle handle = buf->get_handle();
    
    auto thread_worker = [&]() 
    {
        for (int i = 0; i < 100; ++i)
        {
            adam::buffer* resolved = mgr.resolve_handle(handle);
            if (resolved)
            {
                EXPECT_EQ(resolved->get_capacity(), buf->get_capacity());
                resolved->release();
            }
        }
    };
    
    std::thread t1(thread_worker);
    std::thread t2(thread_worker);
    std::thread t3(thread_worker);
    
    t1.join();
    t2.join();
    t3.join();
    
    buf->release();
}

/** @brief Tests that multiple threads can request and return memory safely without race conditions. */
TEST_F(buffer_manager_test, multithreading)
{
    auto& mgr = adam::buffer_manager::get();
    
    auto thread_worker = [&]() 
    {
        std::vector<adam::buffer*> local_buffers;
        
        // Request a batch that forces crossing the max_thread_cache_size limit (64)
        // This ensures the thread drops down into the spinlocked global pool batch transferring
        for (int i = 0; i < 200; ++i)
        {
            adam::buffer* b = mgr.request_buffer(512);
            if (b) local_buffers.push_back(b);
        }
        
        EXPECT_EQ(local_buffers.size(), 200u);

        // Return them all, which will force batch returns to the global pool
        for (adam::buffer* b : local_buffers)
        {
            b->release();
        }
    };

    std::thread t1(thread_worker);
    std::thread t2(thread_worker);
    std::thread t3(thread_worker);

    t1.join();
    t2.join();
    t3.join();
}

/** @brief Benchmark test comparing buffer_manager to standard malloc/free. */
TEST_F(buffer_manager_test, benchmark_vs_malloc)
{
    auto& mgr = adam::buffer_manager::get();
    const size_t num_iterations = 5000000;
    const uint32_t alloc_size = 512;

    volatile void* sink_ptr = nullptr;

    // Benchmark buffer_manager
    auto start_mgr = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iterations; ++i) {
        adam::buffer* buf = mgr.request_buffer(alloc_size);
        sink_ptr = buf;
        if (buf) {
            static_cast<volatile uint8_t*>(buf->data())[0] = static_cast<uint8_t>(i & 0xFF);
            buf->release();
        }
    }
    auto end_mgr = std::chrono::high_resolution_clock::now();
    auto duration_mgr = std::chrono::duration_cast<std::chrono::microseconds>(end_mgr - start_mgr).count();

    // Benchmark malloc/free
    auto start_malloc = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_iterations; ++i) {
        void* ptr = std::malloc(alloc_size);
        sink_ptr = ptr;
        if (ptr) {
            static_cast<volatile uint8_t*>(ptr)[0] = static_cast<uint8_t>(i & 0xFF);
            std::free(ptr);
        }
    }
    auto end_malloc = std::chrono::high_resolution_clock::now();
    auto duration_malloc = std::chrono::duration_cast<std::chrono::microseconds>(end_malloc - start_malloc).count();

    (void)sink_ptr;

    std::cout << "[          ] buffer_manager duration: " << duration_mgr << " us\n";
    std::cout << "[          ] std::malloc duration:    " << duration_malloc << " us\n";

    if (duration_mgr > 0) {
        double times = static_cast<double>(duration_malloc) / static_cast<double>(duration_mgr);
        std::cout << "[          ] -> buffer_manager is " << std::fixed << std::setprecision(2) << times << " times faster than std::malloc\n";
    }

    EXPECT_GT(duration_mgr, 0);
}

TEST_F(buffer_manager_test, referenced_buffer_basic)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* parent = mgr.request_buffer(256);
    adam::buffer* child = mgr.request_buffer(256);
    
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    
    parent->set_referenced_buffer(child);
    
    EXPECT_EQ(parent->get_referenced_buffer(), child);
    
    child->release(); // Release local ownership, parent still owns it
    
    // Verify child is still alive via parent's reference
    EXPECT_EQ(child->get_ref_count(), 1);
    
    parent->release(); // Should release parent and recursively release child
    
    // Verify both are now free
    EXPECT_EQ(parent->get_ref_count(), adam::buffer_manager::buffer_free_state);
    EXPECT_EQ(child->get_ref_count(), adam::buffer_manager::buffer_free_state);
}

TEST_F(buffer_manager_test, referenced_buffer_resolve_recursive)
{
    auto& mgr = adam::buffer_manager::get();
    
    adam::buffer* parent = mgr.request_buffer(256);
    adam::buffer* child = mgr.request_buffer(256);
    
    const char* child_payload = "Child buffer payload";
    child->fill_data(child_payload, static_cast<uint32_t>(strlen(child_payload) + 1));
    
    parent->set_referenced_buffer(child);
    child->release();
    
    EXPECT_EQ(parent->get_ref_count(), 1);
    EXPECT_EQ(child->get_ref_count(), 1);
    
    adam::buffer_handle parent_handle = parent->get_handle();
    
    adam::buffer* resolved_parent = mgr.resolve_handle(parent_handle);
    ASSERT_NE(resolved_parent, nullptr);
    resolved_parent->add_ref(); // Resolving does not automatically add a reference, so we do it here to keep it alive for the test
    
    adam::buffer* resolved_child = resolved_parent->get_referenced_buffer();
    ASSERT_NE(resolved_child, nullptr);
    EXPECT_STREQ(resolved_child->get_data_as<char>(), child_payload);
    
    // The resolved_parent should have called add_ref on the resolved_child
    EXPECT_EQ(child->get_ref_count(), 2);
    EXPECT_EQ(parent->get_ref_count(), 2);
    EXPECT_EQ(resolved_child->get_ref_count(), 2);
    EXPECT_EQ(resolved_parent->get_ref_count(), 2);
    
    resolved_parent->release(); // Releases both parent and child surrogates
    
    // Child's ref count drops back to 1 (owned solely by original parent)
    EXPECT_EQ(child->get_ref_count(), 1);
    EXPECT_EQ(parent->get_ref_count(), 1);
    
    parent->release();
    
    // Verify both are now free
    EXPECT_EQ(parent->get_ref_count(), adam::buffer_manager::buffer_free_state);
    EXPECT_EQ(child->get_ref_count(), adam::buffer_manager::buffer_free_state);
}