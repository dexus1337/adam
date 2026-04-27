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
    std::thread consumer([&]() 
    {
        pop_success = memopentest->pop(received_cmd, std::chrono::milliseconds(500)); // Wait 500ms
    });

    // Small sleep to prove the consumer is actually waiting/blocking
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(cmdcreatetest->push(sent_cmd));

    // 5. Join the thread and verify results
    if (consumer.joinable())
        consumer.join();

    ASSERT_TRUE(pop_success);
    EXPECT_EQ(received_cmd.get_type(), sent_cmd.get_type());
}

/** @brief Tests sending multiple commands in sequence */
TEST_F(command_queue_test, multiple_commands_fifo_order)
{
    const uint32_t queue_size = 100;
    const uint32_t num_commands = 10;
    
    ASSERT_TRUE(cmdcreatetest->create(queue_size));
    ASSERT_TRUE(memopentest->open());

    std::vector<adam::command> received_list;
    std::atomic<bool> consumer_finished{false};

    // 1. Launch Consumer Thread
    std::thread consumer([&]() 
    {
        for (uint32_t i = 0; i < num_commands; ++i) 
        {
            adam::command cmd;
            // Using a generous timeout per command to avoid flakiness
            if (memopentest->pop(cmd, std::chrono::seconds(1))) 
                received_list.push_back(cmd);
        }
        consumer_finished = true;
    });

    // 2. Producer pushes 10 different commands
    for (uint32_t i = 0; i < num_commands; ++i) 
    {
        // Assuming your command constructor or a setter can take a unique ID
        // I'll use different types or IDs to verify order
        adam::command cmd(static_cast<adam::command::type>(i)); 
        ASSERT_TRUE(cmdcreatetest->push(cmd));
    }

    // 3. Cleanup
    if (consumer.joinable())
        consumer.join();

    // 4. Verify results
    ASSERT_TRUE(consumer_finished);
    ASSERT_EQ(received_list.size(), num_commands);

    for (uint32_t i = 0; i < num_commands; ++i) 
    {
        // Verify that command 0 came first, then 1, etc.
        EXPECT_EQ(received_list[i].get_type(), static_cast<adam::command::type>(i)) 
            << "Command at index " << i << " is out of order!";
    }
}