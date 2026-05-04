#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>

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