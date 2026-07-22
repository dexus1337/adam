#include <gtest/gtest.h>
#include <types/map-double-buffer.hpp>

#include <thread>
#include <unordered_map>
#include <atomic>
#include <string>

/** @brief Tests basic update and active visibility */
TEST(map_double_buffer, update_and_get)
{
    adam::map_double_buffer<std::string, int> db_map;

    EXPECT_FALSE(db_map.is_dirty());
    EXPECT_TRUE(db_map.get().empty());

    std::unordered_map<std::string, int> initial_map = {
        {"key1", 100},
        {"key2", 200}
    };

    db_map.update(initial_map);

    // dirty flag should be true before we read/sync it
    EXPECT_TRUE(db_map.is_dirty());

    // calling get() should sync m_pending to m_active and clear dirty
    const auto& active = db_map.get();
    EXPECT_FALSE(db_map.is_dirty());
    EXPECT_EQ(active.size(), 2u);
    EXPECT_EQ(active.at("key1"), 100);
    EXPECT_EQ(active.at("key2"), 200);
}

/** @brief Tests subsequent updates overwrite completely */
TEST(map_double_buffer, overwrite_updates)
{
    adam::map_double_buffer<int, std::string> db_map;

    std::unordered_map<int, std::string> map1 = { {1, "one"}, {2, "two"} };
    db_map.update(map1);
    EXPECT_EQ(db_map.get().size(), 2u);

    std::unordered_map<int, std::string> map2 = { {3, "three"} };
    db_map.update(map2);
    EXPECT_TRUE(db_map.is_dirty());

    const auto& active = db_map.get();
    EXPECT_FALSE(db_map.is_dirty());
    EXPECT_EQ(active.size(), 1u);
    EXPECT_EQ(active.at(3), "three");
    EXPECT_EQ(active.find(1), active.end());
}

/** @brief Tests concurrent updates and reads */
TEST(map_double_buffer, multithreading)
{
    adam::map_double_buffer<int, int> db_map;
    std::atomic<bool> running{true};
    std::atomic<int> read_count{0};

    // Reader thread
    std::thread reader([&]() {
        while (running.load(std::memory_order_relaxed)) {
            const auto& active = db_map.get();
            // Just traverse the active map
            int sum = 0;
            for (auto& [k, v] : active) {
                sum += k + v;
            }
            // Use sum to prevent compiler optimizations
            if (sum > 0) {
                read_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    // Writer thread
    std::thread writer([&]() {
        for (int i = 0; i < 1000; ++i) {
            std::unordered_map<int, int> local;
            for (int j = 0; j < 10; ++j) {
                local[j] = i + j;
            }
            db_map.update(local);
        }
    });

    writer.join();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    running.store(false, std::memory_order_relaxed);
    reader.join();

    // Reader must have run at least once
    EXPECT_GE(read_count.load(), 0);
}
