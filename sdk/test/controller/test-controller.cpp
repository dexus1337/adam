#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <commander/commander.hpp>
#include <commander/messages/command.hpp>
#include <commander/messages/response.hpp>

/** @brief Tests controller's ability to process and send initial data. */
TEST(controller, acquire_initial_data)
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

    cmd.destroy();
    ctrl.destroy();
}

/** @brief Tests controller's ability to receive extended commands */
TEST(controller, receive_extended_commands)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    bool handler_called = false;
    int received_commands = 0;

    // Register a custom test handler (type 9998)
    ctrl.dispatcher().register_handler(9998, [&](const adam::command*, size_t count, adam::command_context& ctx) 
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
    EXPECT_EQ(received_commands, 3);

    cmd.destroy();
    ctrl.destroy();
}

/** @brief Tests controller's ability to send out extended responses over the queue */
TEST(controller, receive_extended_responses)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    // Register a custom test handler (type 9997)
    ctrl.dispatcher().register_handler(9997, [&](const adam::command*, size_t, adam::command_context& ctx) 
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

    cmd.destroy();
    ctrl.destroy();
}