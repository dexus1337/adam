#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <commander/commander.hpp>
#include <commander/messages/command.hpp>
#include <commander/messages/response.hpp>

/** @brief Tests controller's ability to receive extended commands */
TEST(controller, receive_extended_commands)
{
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));

    bool handler_called = false;
    int received_commands = 0;

    // Register a custom test handler (type 255)
    ctrl.dispatcher().register_handler(255, [&](const adam::command*, size_t count, adam::command_context&) -> adam::response 
    {
        handler_called = true;
        received_commands += count;
        return adam::response(adam::response_status::success);
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
    adam::command c1(static_cast<adam::command_type>(255));
    adam::command c2(static_cast<adam::command_type>(255));
    adam::command c3(static_cast<adam::command_type>(255));
    
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