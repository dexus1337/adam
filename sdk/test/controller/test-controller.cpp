#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <commander/commander.hpp>
#include <commander/messages/command.hpp>
#include <commander/messages/response.hpp>
#include <memory/buffer/buffer-manager.hpp>
#include <thread>
#include <chrono>

class testable_controller : public adam::controller
{
public:
    static adam::controller::status req_mq(adam::controller::master_queue_request mqr)
    {
        return request_master_queue(mqr);
    }

    using master_queue = adam::controller::master_queue;
    using queue_master_request_data = adam::controller::queue_master_request_data;

    static const char* get_master_queue_name() { return master_queue_name; }
};

class controller_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        adam::controller::get().get_registry().clear();
    }

    void TearDown() override
    {
        adam::controller::get().get_registry().clear();
        adam::controller::get().destroy();
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests controller's ability to process and send initial data. */
TEST_F(controller_test, acquire_initial_data)
{
    adam::controller& ctrl = adam::controller::get();
    ctrl.set_language(adam::language_german);
    ASSERT_TRUE(ctrl.run(true));

    adam::commander cmd;
    ASSERT_TRUE(cmd.connect()); // connect() implicitly requests initial data

    // Verify initial data was received during connect
    EXPECT_EQ(cmd.get_language(), adam::language_german);

    // Modify language and explicitly request initial data again to verify update
    ctrl.set_language(adam::language_english);
    EXPECT_EQ(cmd.request_initial_data(), adam::response_status::success);
    EXPECT_EQ(cmd.get_language(), adam::language_english);
}

/** @brief Tests controller's ability to receive extended commands */
TEST_F(controller_test, receive_extended_commands)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    bool handler_called = false;
    size_t received_commands = 0;

    const adam::command_type custom_cmd_type = static_cast<adam::command_type>(9998);

    // Register a custom test handler (type 9998)
    ctrl.dispatcher().register_handler(custom_cmd_type, [&](const adam::command*, size_t count, adam::command_context& ctx) 
    {
        handler_called = true;
        received_commands += count;
        ctx.set_single_response_status(adam::response_status::success);
    });

    // Use commander to cleanly establish the queue connection to the controller
    adam::commander cmd;
    ASSERT_TRUE(cmd.connect());

    // Manually open the slave queue to bypass commander's single-command send logic
    adam::queue_shared_duplex<adam::command, adam::response> test_queue(
        adam::string_hashed(std::string("adam::controller_queue_command_") + std::to_string(adam::os::get_current_thread_id()))
    );
    ASSERT_TRUE(test_queue.open());

    // Send 3 commands (2 extended, 1 final)
    adam::command c1(static_cast<adam::command_type>(9998));
    adam::command c2(static_cast<adam::command_type>(9998));
    adam::command c3(static_cast<adam::command_type>(9998));
    
    c1.set_extended(true);
    c2.set_extended(true);

    EXPECT_TRUE(test_queue.request_queue().push(c1));
    EXPECT_TRUE(test_queue.request_queue().push(c2));
    EXPECT_TRUE(test_queue.request_queue().push(c3));

    adam::response resp;
    EXPECT_TRUE(test_queue.response_queue().pop(resp, 1000));
    EXPECT_TRUE(handler_called);
    EXPECT_EQ(received_commands, 3ull);
}

/** @brief Tests controller's ability to send out extended responses over the queue */
TEST_F(controller_test, receive_extended_responses)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    const adam::command_type custom_cmd_type = static_cast<adam::command_type>(9997);

    // Register a custom test handler (type 9997)
    ctrl.dispatcher().register_handler(custom_cmd_type, [&](const adam::command*, size_t, adam::command_context& ctx) 
    {
        ctx.set_single_response_status(adam::response_status::success);
        ctx.responses.front().set_extended(true);

        ctx.responses[1].set_extended(true);
        ctx.responses[1].type() = adam::response_status::success;

        ctx.responses[2].set_extended(false);
        ctx.responses[2].type() = adam::response_status::success;
    });

    // Use commander to cleanly establish the queue connection to the controller
    adam::commander cmd;
    ASSERT_TRUE(cmd.connect());

    // Manually open the slave queue to bypass commander's single-command send logic
    adam::queue_shared_duplex<adam::command, adam::response> test_queue(
        adam::string_hashed(std::string("adam::controller_queue_command_") + std::to_string(adam::os::get_current_thread_id()))
    );
    ASSERT_TRUE(test_queue.open());

    adam::command c(static_cast<adam::command_type>(9997));
    EXPECT_TRUE(test_queue.request_queue().push(c));

    adam::response resp1, resp2, resp3;
    EXPECT_TRUE(test_queue.response_queue().pop(resp1, 1000));
    EXPECT_TRUE(test_queue.response_queue().pop(resp2, 1000));
    EXPECT_TRUE(test_queue.response_queue().pop(resp3, 1000));

    EXPECT_TRUE(resp1.is_extended());
    EXPECT_TRUE(resp2.is_extended());
    EXPECT_FALSE(resp3.is_extended());
}

/** @brief Tests creating duplicate slave queues */
TEST_F(controller_test, duplicate_slave_queue_creation)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    adam::commander cmd;
    ASSERT_TRUE(cmd.connect());

    // Try to request the same queue again
    EXPECT_EQ(testable_controller::req_mq(adam::controller::request_command), adam::controller::status_queue_existing);
    EXPECT_EQ(testable_controller::req_mq(adam::controller::request_event), adam::controller::status_queue_existing);
}

/** @brief Tests destroying nonexistent slave queue */
TEST_F(controller_test, destroy_nonexistent_slave_queue)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    EXPECT_EQ(testable_controller::req_mq(adam::controller::request_command_destroy), adam::controller::status_queue_not_existing);
    EXPECT_EQ(testable_controller::req_mq(adam::controller::request_event_destroy), adam::controller::status_queue_not_existing);
}

/** @brief Tests unauthorized request to master queue */
TEST_F(controller_test, unauthorized_master_queue_request)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    testable_controller::master_queue mq{adam::string_hashed(testable_controller::get_master_queue_name())};
    ASSERT_TRUE(mq.open());

    testable_controller::queue_master_request_data data;
    data.tid = adam::os::get_current_thread_id();
    data.queue = adam::controller::request_command;
    data.code = 0xDEADBEEF; // Invalid secret

    adam::controller::status resp = adam::controller::status_unavailable;
    EXPECT_TRUE(mq.post_request(data, resp, 300));
    EXPECT_EQ(resp, adam::controller::status_unauthorized);

    mq.destroy();
}

/** @brief Tests unknown master queue request */
TEST_F(controller_test, unknown_master_queue_request)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    EXPECT_EQ(testable_controller::req_mq(static_cast<adam::controller::master_queue_request>(999)), adam::controller::status_unknown_master_request);
}

/** @brief Tests context management */
TEST_F(controller_test, context_management)
{
    adam::controller& ctrl = adam::controller::get();
    
    // By default, get_command_ctx() should return the internal default context
    EXPECT_EQ(ctrl.get_command_ctx(), &ctrl.get_default_command_ctx());
    
    std::vector<adam::response> dummy_responses;
    adam::command_context ctx { adam::os::get_current_thread_id(), ctrl.get_registry(), ctrl, dummy_responses, {} };
    
    ctrl.set_command_ctx(&ctx);
    EXPECT_EQ(ctrl.get_command_ctx(), &ctx);
    
    // Clearing the context should fall back to the default context
    ctrl.set_command_ctx(nullptr);
    EXPECT_EQ(ctrl.get_command_ctx(), &ctrl.get_default_command_ctx());
}