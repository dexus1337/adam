#include <gtest/gtest.h>
#include <memory/buffer/buffer.hpp>
#include <memory/buffer/buffer-manager.hpp>

class buffer_test : public ::testing::Test
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

/** @brief Tests basic buffer properties and accessibility after creation. */
TEST_F(buffer_test, properties)
{
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(1024);
    ASSERT_NE(buf, nullptr);

    EXPECT_GE(buf->get_capacity(), 1024u);
    EXPECT_NE(buf->get_data(), nullptr);

    // Verify we can write to the memory safely
    uint64_t* data = static_cast<uint64_t*>(buf->get_data());
    *data = 0xDEADBEEF1337BABE;
    EXPECT_EQ(*data, 0xDEADBEEF1337BABE);

    buf->release();
}

/** @brief Tests generating the IPC-safe handle POD struct. */
TEST_F(buffer_test, get_handle)
{
    adam::buffer* buf = adam::buffer_manager::get().request_buffer(512);
    ASSERT_NE(buf, nullptr);

    adam::buffer_handle handle = buf->get_handle();
    
    EXPECT_NE(handle.memory_index, 0u);
    EXPECT_EQ(handle.size, buf->get_capacity());
    EXPECT_GE(handle.offset, 0u);

    buf->release();
}

/** @brief Tests the shared reference counting mechanics. */
TEST_F(buffer_test, ref_counting)
{
    adam::buffer* buf1 = adam::buffer_manager::get().request_buffer(256);
    ASSERT_NE(buf1, nullptr);

    // Add a reference, making the count 2
    buf1->add_ref();
    
    // Release once, count drops to 1, should NOT be returned to the pool yet
    buf1->release();

    // Requesting another buffer of same size shouldn't yield buf1, because it's still active
    adam::buffer* buf2 = adam::buffer_manager::get().request_buffer(256);
    EXPECT_NE(buf1, buf2);

    // Final releases return them to pool
    buf1->release();
    buf2->release();
}