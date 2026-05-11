#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>
#include <commander/command-response/command.hpp>
#include <commander/command-response/response.hpp>
#include <thread>

/** @brief Tests commander command queue lifecycle */
TEST(commander, command_queue_lifecycle)
{
    // Ensure the controller is running asynchronously so the master queue can respond
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));
    
    adam::commander cmd;
    
    EXPECT_FALSE(cmd.is_active());
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(cmd.connect());
    EXPECT_TRUE(cmd.is_active());
    
    // Disconnect and clean up
    EXPECT_TRUE(cmd.destroy());
    EXPECT_FALSE(cmd.is_active());

    // Clean up controller for subsequent tests
    EXPECT_TRUE(ctrl.destroy());
}

class testable_commander : public adam::commander
{
public:
    using adam::commander::send_command;
    using adam::commander::m_queue_command;
};

/** @brief Tests commander's ability to receive extended responses */
TEST(commander, receive_extended_responses)
{
    testable_commander cmd;
    
    // We manually open a queue to mock the controller's side of the connection.
    adam::string_hashed queue_name(std::string("adam::controller_queue_command_") + std::to_string(adam::os::get_current_thread_id()));
    adam::queue_shared_duplex<adam::command, adam::response> mock_controller_queue(queue_name);
    
    ASSERT_TRUE(mock_controller_queue.create(100));
    
    // Connect our commander to this pre-existing queue manually
    ASSERT_TRUE(cmd.m_queue_command.open());

    // We need a background thread to act as the controller and send extended responses
    std::thread mock_controller_thread([&]() {
        adam::command incoming_cmd;
        if (mock_controller_queue.request_queue().pop(incoming_cmd, 1000))
        {
            adam::response r1(adam::response_status::success);
            adam::response r2(adam::response_status::success);
            adam::response r3(adam::response_status::success);
            
            r1.set_extended(true);
            r2.set_extended(true);
            
            mock_controller_queue.response_queue().push(r1);
            mock_controller_queue.response_queue().push(r2);
            mock_controller_queue.response_queue().push(r3);
        }
    });

    adam::command test_cmd(adam::command_type::invalid);
    adam::response* resp_array = nullptr;
    
    adam::response_status status = cmd.send_command(test_cmd, &resp_array);
    
    EXPECT_EQ(status, adam::response_status::success);
    ASSERT_NE(resp_array, nullptr);
    
    // Verify that the array actually contains our extended responses
    EXPECT_TRUE(resp_array[0].is_extended());
    EXPECT_TRUE(resp_array[1].is_extended());
    EXPECT_FALSE(resp_array[2].is_extended());

    mock_controller_thread.join();
    
    cmd.m_queue_command.disable();
    mock_controller_queue.destroy();
}