#include <gtest/gtest.h>
#include <types/queue-shared.hpp>

#include <chrono>
#include <thread>

class queue_shared_test : public ::testing::Test
{
protected:

    void SetUp() override
    {
        auto name = adam::string_hashed("adam::queue_shared_test");

        queuecreatetest   = new adam::queue_shared<int>(name);
        queueopentest     = new adam::queue_shared<int>(name);
    }

    void TearDown() override
    {
        queuecreatetest->destroy();
        queueopentest->destroy();

        delete queuecreatetest;
        delete queueopentest;
    }

    adam::queue_shared<int>* queuecreatetest;
    adam::queue_shared<int>* queueopentest;
};


/** @brief Tests if command queues and be created and openend, and that the maximal command count is properly synched */
TEST_F(queue_shared_test, create_open)
{
    ASSERT_TRUE(queuecreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(queueopentest->open());           // Try to open this queue
    
    EXPECT_TRUE(queuecreatetest->is_empty());
    EXPECT_FALSE(queuecreatetest->is_full());

    EXPECT_GE(queueopentest->get_max_items(), 100u);
    EXPECT_TRUE(queueopentest->is_empty());
    EXPECT_FALSE(queueopentest->is_full());
}

/** @brief Tests if command queues is properly reacting when its full */
TEST_F(queue_shared_test, full_lifecycle)
{
    uint32_t size = 2; 
    ASSERT_TRUE(queuecreatetest->create(size));    
    ASSERT_TRUE(queueopentest->open());         
    
    // 1. Verify Initial State
    EXPECT_TRUE(queuecreatetest->is_empty());
    EXPECT_FALSE(queuecreatetest->is_full());

    // 2. Fill the queue to its capacity
    int item = 1337;
    
    // First push should succeed
    EXPECT_TRUE(queuecreatetest->push(item));
    
    // 3. Verify Full State
    // Now that 1 command is in (and 1 slot is reserved for the tail), 
    // the queue should report full.
    EXPECT_FALSE(queuecreatetest->is_empty());
    EXPECT_TRUE(queuecreatetest->is_full());

    // 4. Verify Blocked Push
    // The second push should fail because the queue is full
    EXPECT_FALSE(queuecreatetest->push(item));

    // 5. Clear the queue and verify it recovers
    int out_item = 0;
    EXPECT_TRUE(queueopentest->pop(out_item, std::chrono::milliseconds(100)));
    
    EXPECT_TRUE(queuecreatetest->is_empty());
    EXPECT_FALSE(queuecreatetest->is_full());

    // 6. Check reset
    // This push should succeed as there is place in the queue again
    EXPECT_TRUE(queuecreatetest->push(item));

    EXPECT_EQ(out_item, item);
}

/** @brief Tests the entire functionality with a single command */
TEST_F(queue_shared_test, single_command_send_and_recievied)
{
    ASSERT_TRUE(queuecreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(queueopentest->open());           // Try to open this queue
    
    int sent_item       = 1337;
    int received_item   = 0;
    bool pop_success    = false;

    // We use a thread to simulate the "external process" behavior
    std::thread consumer([&]() 
    {
        pop_success = queueopentest->pop(received_item, std::chrono::milliseconds(500)); // Wait 500ms
    });

    // Small sleep to prove the consumer is actually waiting/blocking
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(queuecreatetest->push(sent_item));

    // 5. Join the thread and verify results
    if (consumer.joinable())
        consumer.join();

    ASSERT_TRUE(pop_success);
    EXPECT_EQ(received_item, sent_item);
}

/** @brief Tests sending multiple commands in sequence */
TEST_F(queue_shared_test, multiple_commands_fifo_order)
{
    const uint32_t queue_size = 100;
    const uint32_t num_commands = 10;
    
    ASSERT_TRUE(queuecreatetest->create(queue_size));
    ASSERT_TRUE(queueopentest->open());

    std::vector<int> received_list;
    std::atomic<bool> consumer_finished{false};

    // 1. Launch Consumer Thread
    std::thread consumer([&]() 
    {
        for (uint32_t i = 0; i < num_commands; ++i) 
        {
            int item;
            // Using a generous timeout per command to avoid flakiness
            if (queueopentest->pop(item, std::chrono::seconds(1))) 
                received_list.push_back(item);
        }
        consumer_finished = true;
    });

    // 2. Producer pushes 10 different commands
    for (uint32_t i = 0; i < num_commands; ++i) 
    {
        // Assuming your command constructor or a setter can take a unique ID
        // I'll use different types or IDs to verify order
        int item = static_cast<int>(i);
        ASSERT_TRUE(queuecreatetest->push(item));
    }

    // 3. Cleanup
    if (consumer.joinable())
        consumer.join();

    // 4. Verify results
    ASSERT_TRUE(consumer_finished);
    ASSERT_EQ(received_list.size(), num_commands);

    for (int i = 0; i < static_cast<int>(num_commands); ++i) 
    {
        // Verify that command 0 came first, then 1, etc.
        EXPECT_EQ(received_list[i], i)
            << "Command at index " << i << " is out of order!";
    }
}