#include <gtest/gtest.h>
#include <memory/memory.hpp>


/** @brief Tests the initialization and accessability of memory objects. */
TEST(memory, new)
{
    adam::memory memtest(adam::string_hashed("adam::memory_test1"));

    ASSERT_TRUE(memtest.create(1024)); // Create a shared memory segment of 1KB

    EXPECT_TRUE(memtest.get() != nullptr); // Ensure the base pointer is valid

    ASSERT_GE(memtest.get_size(), 1024u); // Automatic alignment to page, should be greater than 1kb on any system

    static constexpr uint64_t testval = 0x1337133813391331;

    *reinterpret_cast<uint64_t*>(memtest.get()) = testval;

    memtest.destroy(); // Clean up the shared memory segment
}

/** @brief Tests opening an existing region */
TEST(memory, existing)
{
    auto name = adam::string_hashed("adam::memory_test2");

    adam::memory memtest(name);

    ASSERT_TRUE(memtest.create(1024)); // Create a shared memory segment of 1KB

    static constexpr uint64_t testval = 0x1337133813391331;

    *reinterpret_cast<uint64_t*>(memtest.get()) = testval;

    adam::memory accessmemtest(name);

    ASSERT_TRUE(accessmemtest.open());
    EXPECT_EQ(accessmemtest.get_size(), memtest.get_size());

    EXPECT_EQ(*reinterpret_cast<uint64_t*>(accessmemtest.get()), testval);

    accessmemtest.destroy();

    memtest.destroy(); // Clean up the shared memory segment
}