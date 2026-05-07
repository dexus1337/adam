#include <gtest/gtest.h>
#include <types/queue-shared-duplex.hpp>

#include <chrono>
#include <thread>

class queue_shared_duplex_test : public ::testing::Test
{
protected:

    void SetUp() override
    {
        auto name = adam::string_hashed("adam::queue_shared_duplex_test");

        queuecreatetest   = new adam::queue_shared_duplex<int, int>(name);
        queueopentest     = new adam::queue_shared_duplex<int, int>(name);
    }

    void TearDown() override
    {
        queuecreatetest->destroy();
        queueopentest->destroy();

        delete queuecreatetest;
        delete queueopentest;
    }

    adam::queue_shared_duplex<int, int>* queuecreatetest;
    adam::queue_shared_duplex<int, int>* queueopentest;
};

/** @brief Tests if duplex queues and be created and openend, and that the maximal command count is properly synched */
TEST_F(queue_shared_duplex_test, create_open)
{
    ASSERT_TRUE(queuecreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(queueopentest->open());           // Try to open this queue
    
    EXPECT_TRUE(queuecreatetest->request_queue().is_empty());
    EXPECT_FALSE(queuecreatetest->request_queue().is_full());
    EXPECT_TRUE(queuecreatetest->response_queue().is_empty());
    EXPECT_FALSE(queuecreatetest->response_queue().is_full());

    EXPECT_GE(queueopentest->get_max_items(), 100u);
    
    EXPECT_TRUE(queueopentest->request_queue().is_empty());
    EXPECT_FALSE(queueopentest->request_queue().is_full());
    EXPECT_TRUE(queueopentest->response_queue().is_empty());
    EXPECT_FALSE(queueopentest->response_queue().is_full());
}

/** @brief Tests the entire functionality with a single command */
TEST_F(queue_shared_duplex_test, single_command_send_and_recievied)
{
    ASSERT_TRUE(queuecreatetest->create(100));    // Create a queue with 100 max commands
    ASSERT_TRUE(queueopentest->open());           // Try to open this queue
    
    int req             = 1337;
    int res             = 1338;
    int received_req    = 0;
    int received_res    = 0;
    bool pop_success    = false;

    // We use a thread to simulate the "external process" behavior
    std::thread consumer([&]() 
    {
        pop_success = queueopentest->request_queue().pop(received_req, std::chrono::milliseconds(500)); // Wait 500ms
        queueopentest->response_queue().push(res);
    });

    ASSERT_TRUE(queuecreatetest->post_request(req, received_res, std::chrono::milliseconds(100)));

    // 5. Join the thread and verify results
    if (consumer.joinable())
        consumer.join();

    ASSERT_TRUE(pop_success);
    EXPECT_EQ(received_req, req);
    EXPECT_EQ(received_res, res);
}
