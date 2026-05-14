#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/registry-module-manager.hpp>
#include <controller/registry.hpp>

class registry_module_manager_test : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/** @brief Tests that scanning handles non-existent directories gracefully. */
TEST_F(registry_module_manager_test, scan_for_modules_invalid_dir)
{
    adam::controller& ctrl = adam::controller::get();
    
    ctrl.get_registry().add_module_path(adam::string_hashed("/invalid_adam_directory_that_does_not_exist_1337"));

    // Should handle non-existent directories gracefully and return false since no changes were made to the module cache
    EXPECT_FALSE(ctrl.get_registry().modules().scan_for_modules());
}

/** @brief Tests attempting to load a module that has not been scanned or doesn't exist. */
TEST_F(registry_module_manager_test, load_module_invalid)
{
    adam::controller& ctrl = adam::controller::get();
    
    adam::string_hashed invalid_mod_name("invalid_module");
    EXPECT_FALSE(ctrl.get_registry().modules().load_module(invalid_mod_name));
    EXPECT_EQ(ctrl.get_registry().modules().get_loaded_module(invalid_mod_name), nullptr);
}

/** @brief Tests that getters for internal module maps return safely without faulting. */
TEST_F(registry_module_manager_test, module_maps_access)
{
    adam::controller& ctrl = adam::controller::get();
    
    const auto& available = ctrl.get_registry().modules().get_available_modules();
    const auto& unavailable = ctrl.get_registry().modules().get_unavailable_modules();
    const auto& loaded = ctrl.get_registry().modules().get_loaded_modules();
    
    EXPECT_GE(available.size(), 0u);
    EXPECT_GE(unavailable.size(), 0u);
    EXPECT_GE(loaded.size(), 0u);
}