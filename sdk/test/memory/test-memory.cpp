#include <gtest/gtest.h>
#include <memory/memory.hpp>
#include <memory> // For std::unique_ptr

// Use a test fixture for setup and teardown
class memory_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mem_owner.reset();
        mem_observer.reset();
    }

    void TearDown() override
    {
        // Ensure cleanup happens even if a test fails
        if (mem_observer)
        {
            mem_observer->destroy();
            mem_observer.reset();
        }
        if (mem_owner)
        {
            mem_owner->destroy();
            mem_owner.reset();
        }
    }

    std::unique_ptr<adam::memory> mem_owner;
    std::unique_ptr<adam::memory> mem_observer;
    const adam::string_hashed test_name = adam::string_hashed("adam::memory_fixture_test");
};


/** @brief Tests basic creation, properties, and destruction of a memory segment. */
TEST_F(memory_test, create_and_destroy)
{
    mem_owner = std::make_unique<adam::memory>(test_name);

    ASSERT_TRUE(mem_owner->create(1024));
    EXPECT_TRUE(mem_owner->is_active());
    EXPECT_TRUE(mem_owner->is_owner());
    EXPECT_NE(mem_owner->get(), nullptr);
    EXPECT_GE(mem_owner->get_size(), 1024u);

    // Write a value to prove we have access
    static constexpr uint64_t test_val = 0xDEADBEEFCAFEBABE;
    *reinterpret_cast<uint64_t*>(mem_owner->get()) = test_val;
    EXPECT_EQ(*reinterpret_cast<uint64_t*>(mem_owner->get()), test_val);

    // TearDown will handle destruction
}

/** @brief Tests opening an existing memory segment and verifying its content. */
TEST_F(memory_test, open_existing)
{
    mem_owner = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_owner->create(1024));

    // Write a value from the owner's side
    static constexpr uint64_t test_val = 0x1337BEEF1337BABE;
    *reinterpret_cast<uint64_t*>(mem_owner->get()) = test_val;

    // A separate process/object opens the same memory
    mem_observer = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_observer->open());
    EXPECT_TRUE(mem_observer->is_active());
    EXPECT_FALSE(mem_observer->is_owner());
    EXPECT_EQ(mem_observer->get_size(), mem_owner->get_size());

    // Verify the observer can read the value written by the owner
    EXPECT_EQ(*reinterpret_cast<uint64_t*>(mem_observer->get()), test_val);
}

/** @brief Tests that creating a memory segment with zero size fails. */
TEST_F(memory_test, create_zero_size)
{
    mem_owner = std::make_unique<adam::memory>(test_name);
    // Creating with zero size should not be allowed as it's ambiguous and
    // can behave differently across platforms.
    EXPECT_FALSE(mem_owner->create(0));
}

/** @brief Tests that opening a non-existent memory segment fails gracefully. */
TEST_F(memory_test, open_non_existent)
{
    mem_observer = std::make_unique<adam::memory>(test_name);
    // Trying to open a segment that was never created should fail.
    EXPECT_FALSE(mem_observer->open());
}

/** @brief Tests the disable() functionality. */
TEST_F(memory_test, disable_flag)
{
    mem_owner = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_owner->create(1024));
    EXPECT_TRUE(mem_owner->is_active());

    mem_owner->disable();
    EXPECT_FALSE(mem_owner->is_active());
}

/** @brief Tests that calling destroy multiple times is safe and handles gracefully. */
TEST_F(memory_test, double_destroy)
{
    mem_owner = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_owner->create(1024));

    // First destroy should clear resources normally
    EXPECT_TRUE(mem_owner->destroy());
    
    // Subsequent destroys should safely return true without segfaults
    EXPECT_TRUE(mem_owner->destroy());
}

/** @brief Tests the ownership and destruction lifecycle between owner and observers. */
TEST_F(memory_test, ownership_and_destruction)
{
    mem_owner = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_owner->create(1024));

    mem_observer = std::make_unique<adam::memory>(test_name);
    ASSERT_TRUE(mem_observer->open());

    // 1. Observer destroying shouldn't unallocate the memory system-wide
    EXPECT_TRUE(mem_observer->destroy());
    mem_observer.reset();

    // 2. Proof: Another observer should STILL be able to open it because the owner is alive
    adam::memory another_observer(test_name);
    EXPECT_TRUE(another_observer.open());
    EXPECT_TRUE(another_observer.destroy());

    // 3. Owner destroys it. This unlinks the OS resource completely
    EXPECT_TRUE(mem_owner->destroy());
    mem_owner.reset();

    // 4. Now a new observer should FAIL to open it
    adam::memory late_observer(test_name);
    EXPECT_FALSE(late_observer.open());
}