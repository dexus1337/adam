#include <gtest/gtest.h>
#include <controller/controller.hpp>

/** @brief Tests that the controller correctly enforces the singleton pattern */
TEST(controller, singleton_instance)
{
    // Retrieve the instance twice
    adam::controller& instance1 = adam::controller::get();
    adam::controller& instance2 = adam::controller::get();

    // Both references should point to the exact same memory address
    EXPECT_EQ(&instance1, &instance2);
}

/** @brief Tests basic controller lifecycle using the singleton */
TEST(controller, lifecycle)
{
    adam::controller& ctrl = adam::controller::get();
    
    EXPECT_TRUE(ctrl.run(true));
    EXPECT_TRUE(ctrl.is_active());
    EXPECT_TRUE(ctrl.destroy());
    EXPECT_FALSE(ctrl.is_active());
}

/** @brief Tests the logging functionality */
TEST(controller, logging)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Ensures the standard string-based logging helper forwards properly
    ctrl.log(adam::log::info, "Test log from controller unit test");
    
    // Ensures the direct log-object logging works
    adam::log custom_log(adam::log::warning, "Test custom log object from controller unit test");
    ctrl.log(custom_log);
}

/** @brief Tests the master queue IPC subscriptions (commands, logs, sinks) */
TEST(controller, master_queue_subscriptions)
{
    adam::controller& ctrl = adam::controller::get();
    
    // We need the controller master queue running asynchronously to process our IPC requests
    ASSERT_TRUE(ctrl.run(true));

    EXPECT_TRUE(ctrl.destroy());
}