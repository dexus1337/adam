#include <gtest/gtest.h>
#include <memory/shared/memory-shared.hpp>


/** @brief Tests the initialization and accessability of memory_shared objects. */
TEST(memory_shared, new)
{
    adam::memory_shared memtest(adam::string_hashed("adam::memory_shared_test1"));

    ASSERT_TRUE(memtest.create(1024)); // Create a shared memory segment of 1KB

    EXPECT_TRUE(memtest.get() != nullptr); // Ensure the base pointer is valid

    ASSERT_GE(memtest.get_size(), 1024u); // Automatic alignment to page, should be greater than 1kb on any system

    static constexpr uint64_t testval = 0x1337133813391331;

    *reinterpret_cast<uint64_t*>(memtest.get()) = testval;

    memtest.destroy(); // Clean up the shared memory segment
}

/** @brief Tests opening an existing region */
TEST(memory_shared, existing)
{
    auto name = adam::string_hashed("adam::memory_shared_test2");

    adam::memory_shared memtest(name);

    ASSERT_TRUE(memtest.create(1024)); // Create a shared memory segment of 1KB

    static constexpr uint64_t testval = 0x1337133813391331;

    *reinterpret_cast<uint64_t*>(memtest.get()) = testval;

    adam::memory_shared accessmemtest(name);

    ASSERT_TRUE(accessmemtest.open());
    EXPECT_EQ(accessmemtest.get_size(), memtest.get_size());

    EXPECT_EQ(*reinterpret_cast<uint64_t*>(accessmemtest.get()), testval);

    accessmemtest.destroy();

    memtest.destroy(); // Clean up the shared memory segment
}