#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/controller-module-manager.hpp>

class controller_module_manager_test : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/** @brief Tests that scanning handles non-existent directories gracefully. */
TEST_F(controller_module_manager_test, scan_for_modules_invalid_dir)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Should handle non-existent directories gracefully and return true (yielding an empty scan)
    EXPECT_TRUE(ctrl.modules().scan_for_modules("/invalid_adam_directory_that_does_not_exist_1337"));
}

/** @brief Tests attempting to load a module that has not been scanned or doesn't exist. */
TEST_F(controller_module_manager_test, load_module_invalid)
{
    adam::controller& ctrl = adam::controller::get();
    
    adam::string_hashed invalid_mod_name("invalid_module");
    EXPECT_FALSE(ctrl.modules().load_module(invalid_mod_name));
    EXPECT_EQ(ctrl.modules().get_loaded_module(invalid_mod_name), nullptr);
}

/** @brief Tests that getters for internal module maps return safely without faulting. */
TEST_F(controller_module_manager_test, module_maps_access)
{
    adam::controller& ctrl = adam::controller::get();
    
    const auto& available = ctrl.modules().get_available_modules();
    const auto& unavailable = ctrl.modules().get_unavailable_modules();
    const auto& loaded = ctrl.modules().get_loaded_modules();
    
    EXPECT_GE(available.size(), 0u);
    EXPECT_GE(unavailable.size(), 0u);
    EXPECT_GE(loaded.size(), 0u);
}