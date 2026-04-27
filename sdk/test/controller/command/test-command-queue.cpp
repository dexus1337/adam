#include <gtest/gtest.h>
#include <controller/command/command-queue.hpp>

#include <chrono>
#include <thread>

class command_queue_test : public ::testing::Test
{
protected:

    void SetUp() override
    {
        auto name = adam::string_hashed("adam::command_queue_test");

        cmdcreatetest = new adam::command_queue(name);
        memopentest = new adam::command_queue(name);
    }

    void TearDown() override
    {
        cmdcreatetest->destroy();
        memopentest->destroy();

        delete cmdcreatetest;
        delete memopentest;
    }

    adam::command_queue* cmdcreatetest;
    adam::command_queue* memopentest;
};


/** @brief Tests if command queues and be created and openend, and that the maximal command count is properly synched */
TEST_F(command_queue_test, create_open_command_count)
{
    ASSERT_TRUE(cmdcreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(memopentest->open());           // Try to open this queue
    
    EXPECT_GE(memopentest->get_max_command_count(), 100u);
}

/** @brief Tests the entire functionality with a single command */
TEST_F(command_queue_test, single_command_send_and_recievied)
{
    ASSERT_TRUE(cmdcreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(memopentest->open());           // Try to open this queue
    
    adam::command sent_cmd(adam::command::login);
    adam::command received_cmd;
    bool pop_success = false;

    // We use a thread to simulate the "external process" behavior
    std::thread consumer([&]() {
        pop_success = memopentest->pop(received_cmd, std::chrono::milliseconds(100)); // Wait 100ms
    });

    // Small sleep to prove the consumer is actually waiting/blocking
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(cmdcreatetest->push(sent_cmd));

    // 5. Join the thread and verify results
    if (consumer.joinable())
        consumer.join();

    ASSERT_TRUE(pop_success) << "Consumer timed out or failed to pop command";
    EXPECT_EQ(received_cmd.get_type(), sent_cmd.get_type());
}