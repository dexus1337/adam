#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>

/** @brief Tests commander is_active after a successful connection. */
TEST(commander, is_active_on_success)
{
    // Ensure the controller is running asynchronously so the master queue can respond
    adam::controller& ctrl = adam::controller::get();
    ASSERT_TRUE(ctrl.run(true));
    
    adam::commander cmdr;
    
    EXPECT_FALSE(cmdr.is_active());
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());
    
    // Disconnect and clean up
    EXPECT_TRUE(cmdr.destroy());
    EXPECT_FALSE(cmdr.is_active());

    // Clean up controller for subsequent tests
    EXPECT_TRUE(ctrl.destroy());
}

/** @brief Tests commander is_active when master queue request fails. */
TEST(commander, is_active_on_failure)
{
    // Ensure the controller is NOT running
    adam::controller& ctrl = adam::controller::get();
    if (ctrl.is_active())
        ctrl.destroy();

    adam::commander cmdr;
    
    EXPECT_FALSE(cmdr.is_active());
    EXPECT_FALSE(cmdr.connect()); // Should fail as controller is not running
    EXPECT_FALSE(cmdr.is_active());
}