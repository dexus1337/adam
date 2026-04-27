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

        memtest = new adam::memory_shared(name);
    }

    void TearDown() override
    {
        memtest->destroy();

        delete memtest;
    }

    adam::memory_shared* memtest;
};


/** @brief Tests the wait without being notified, also tests the timeout */
TEST_F(memory_shared_signal_test, wait_timeout)
{
    ASSERT_TRUE(memtest->create(1024)); // Create a shared memory segment of 1KB

    auto start = std::chrono::steady_clock::now();
    bool result = memtest->signal().wait(100); // Wait for 100ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_FALSE(result); // Should timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 100); // Ensure it actually waited at least 100ms

    memtest->destroy(); // Clean up the shared memory segment
}

/** @brief Tests the notification functionality of memory_shared_signal. */
TEST_F(memory_shared_signal_test, notify_and_wait)
{
    ASSERT_TRUE(memtest->create(1024)); // Create a shared memory segment of 1KB

    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        memtest->signal().notify();
    });

    auto start = std::chrono::steady_clock::now();
    bool result = memtest->signal().wait(500); // Wait for up to 500ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result); // Should be signaled, not timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 45); // Ensure it waited until notified

    notifier.join();
}

/** @brief Tests the notification functionality of memory_shared_signal. */
TEST_F(memory_shared_signal_test, notify_and_wait_std_chrono)
{
    ASSERT_TRUE(memtest->create(1024)); // Create a shared memory segment of 1KB

    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        memtest->signal().notify();
    });

    auto start = std::chrono::steady_clock::now();
    bool result = memtest->signal().wait(std::chrono::milliseconds(500)); // Wait for up to 500ms
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(result); // Should be signaled, not timeout
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 45); // Ensure it waited until notified

    notifier.join();
}