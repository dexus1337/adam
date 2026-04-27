#include <gtest/gtest.h>
#include <memory/shared/memory-shared.hpp>
#include <memory/shared/memory-shared-signal.hpp>

#include <chrono>
#include <thread>

class memory_shared_signal_test : public ::testing::Test
{
protected:

    void SetUp() override
    {
        auto name = adam::string_hashed("adam::memory_shared_signal_test");

        memcreatetest = new adam::memory_shared(name);
        memopentest = new adam::memory_shared(name);
    }

    void TearDown() override
    {
        memcreatetest->destroy();
        memopentest->destroy();

        delete memcreatetest;
        delete memopentest;
    }

    adam::memory_shared* memcreatetest;
    adam::memory_shared* memopentest;
};


/** @brief Tests the wait without being notified, also tests the timeout */
TEST_F(memory_shared_signal_test, wait_timeout)
{
    ASSERT_TRUE(memcreatetest->create(1024)); // Create a shared memory segment of 1KB

    auto start = std::chrono::steady_clock::now();
    bool result = memcreatetest->signal().wait(100); // Wait for 100ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_FALSE(result); // Should timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 100); // Ensure it actually waited at least 100ms
}

/** @brief Tests the notification functionality of memory_shared_signal. */
TEST_F(memory_shared_signal_test, create_notify_and_wait)
{
    ASSERT_TRUE(memcreatetest->create(1024)); // Create a shared memory segment of 1KB

    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        memcreatetest->signal().notify();
    });

    auto start = std::chrono::steady_clock::now();
    bool result = memcreatetest->signal().wait(500); // Wait for up to 500ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result); // Should be signaled, not timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 45); // Ensure it waited until notified

    notifier.join();
}

/** @brief Tests the notification functionality of memory_shared_signal. */
TEST_F(memory_shared_signal_test, create_notify_and_wait_std_chrono)
{
    ASSERT_TRUE(memcreatetest->create(1024)); // Create a shared memory segment of 1KB

    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        memcreatetest->signal().notify();
    });

    auto start = std::chrono::steady_clock::now();
    bool result = memcreatetest->signal().wait(std::chrono::milliseconds(500)); // Wait for up to 500ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result); // Should be signaled, not timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 45); // Ensure it waited until notified

    notifier.join();
}

/** @brief Tests the notification functionality of memory_shared_signal. */
TEST_F(memory_shared_signal_test, open_notify_and_wait)
{
    ASSERT_TRUE(memcreatetest->create(1024)); // Create a shared memory segment of 1KB
    ASSERT_TRUE(memopentest->open());

    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        memcreatetest->signal().notify();
    });

    auto start = std::chrono::steady_clock::now();
    bool result = memopentest->signal().wait(500); // Wait for up to 500ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result); // Should be signaled, not timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 45); // Ensure it waited until notified

    notifier.join();
}
