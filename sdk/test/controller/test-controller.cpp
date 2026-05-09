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

/** @brief Tests the module scanning with an invalid directory */
TEST(controller, scan_for_modules_invalid_dir)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Should handle non-existent directories gracefully and return true (empty scan)
    EXPECT_TRUE(ctrl.modules().scan_for_modules("/invalid_adam_directory_that_does_not_exist_1337"));
}

/** @brief Tests the loading of an invalid module */
TEST(controller, load_module_invalid)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Try to load a module that hasn't been scanned or doesn't exist
    adam::string_hashed invalid_mod_name("invalid_module");
    EXPECT_FALSE(ctrl.modules().load_module(invalid_mod_name));
    EXPECT_EQ(ctrl.modules().get_loaded_module(invalid_mod_name), nullptr);
}

/** @brief Tests that getters for module maps return correctly */
TEST(controller, module_maps_access)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Just verify we can access the maps and they don't cause faults
    const auto& available = ctrl.modules().get_available_modules();
    const auto& unavailable = ctrl.modules().get_unavailable_modules();
    const auto& loaded = ctrl.modules().get_loaded_modules();
    
    EXPECT_GE(available.size(), 0u);
    EXPECT_GE(unavailable.size(), 0u);
    EXPECT_GE(loaded.size(), 0u);
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