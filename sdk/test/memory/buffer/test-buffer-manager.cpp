#include <gtest/gtest.h>
#include <memory/buffer/buffer-manager.hpp>
#include <memory/buffer/buffer.hpp>

#include <thread>
#include <vector>

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