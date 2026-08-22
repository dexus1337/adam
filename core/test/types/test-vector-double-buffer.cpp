#include <gtest/gtest.h>
#include <types/vector-double-buffer.hpp>

#include <thread>
#include <vector>
#include <atomic>

/** @brief Tests basic push_back and visibility after iterate */
TEST(vector_double_buffer, push_and_iterate)
{
    adam::vector_double_buffer<int*> buf;
    int a = 1, b = 2;

    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);

    buf.push_back(&a);
    buf.push_back(&b);

    // Should still be empty on the read side before iterate
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);

    int sum = 0;
    buf.iterate([&](const auto& active) {
        for (auto val : active) {
            sum += *val;
        }
    });

    EXPECT_EQ(sum, 3);
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.size(), 2u);
}

/** @brief Tests element removal */
TEST(vector_double_buffer, remove)
{
    adam::vector_double_buffer<int> buf;
    
    buf.push_back(10);
    buf.push_back(20);
    buf.push_back(30);

    buf.iterate([](const auto&) {}); // Sync
    EXPECT_EQ(buf.size(), 3u);

    EXPECT_TRUE(buf.remove(20));
    EXPECT_FALSE(buf.remove(40)); // Not in buffer

    // Should still be 3 before iterate
    EXPECT_EQ(buf.size(), 3u);

    std::vector<int> found;
    buf.iterate([&](const auto& active) {
        found = active;
    });

    EXPECT_EQ(buf.size(), 2u);
    ASSERT_EQ(found.size(), 2u);
    EXPECT_EQ(found[0], 10);
    EXPECT_EQ(found[1], 30);
}

/** @brief Tests clearing the buffer */
TEST(vector_double_buffer, clear)
{
    adam::vector_double_buffer<int> buf;
    
    buf.push_back(1);
    buf.push_back(2);

    buf.iterate([](const auto&) {});
    EXPECT_EQ(buf.size(), 2u);

    buf.clear();
    EXPECT_EQ(buf.size(), 2u); // Active buffer isn't updated until iterate()

    buf.iterate([](const auto&) {});
    EXPECT_EQ(buf.size(), 0u);
    EXPECT_TRUE(buf.empty());
}

/** @brief Tests reordering of elements */
TEST(vector_double_buffer, reorder)
{
    adam::vector_double_buffer<int> buf;
    
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.iterate([](const auto&) {});

    std::vector<int> new_order = { 3, 1, 2 };
    buf.reorder(new_order);

    std::vector<int> found;
    buf.iterate([&](const auto& active) {
        found = active;
    });

    ASSERT_EQ(found.size(), 3u);
    EXPECT_EQ(found[0], 3);
    EXPECT_EQ(found[1], 1);
    EXPECT_EQ(found[2], 2);
}

/** @brief Tests thread safety with concurrent reads and writes */
TEST(vector_double_buffer, multithreading)
{
    adam::vector_double_buffer<int> buf;
    std::atomic<bool> running{true};
    std::atomic<int> read_sum{0};
    std::atomic<int> read_count{0};

    // Reader thread
    std::thread reader([&]() {
        while (running.load(std::memory_order_relaxed)) {
            buf.iterate([&](const auto& active) {
                int sum = 0;
                for (int val : active) {
                    sum += val;
                }
                read_sum.store(sum, std::memory_order_relaxed);
            });
            read_count.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // Writer thread
    std::thread writer([&]() {
        for (int i = 0; i < 1000; ++i) {
            buf.push_back(i);
            if (i % 10 == 0) {
                buf.remove(i - 5);
            }
        }
    });

    writer.join();
    
    // Let reader do a few more iterations to ensure the last states are caught
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    running.store(false, std::memory_order_relaxed);
    reader.join();

    EXPECT_GT(read_count.load(), 0); // Ensure the reader actually ran
}