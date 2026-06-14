#include <gtest/gtest.h>
#include <controller/controller.hpp>
#include <controller/registry-module-manager.hpp>
#include <controller/registry.hpp>

using namespace adam::string_hashed_ct_literals;

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
    
    // 1. Save original configuration and loaded module names
    auto* paths_list = ctrl.get_registry().get_module_paths();
    std::vector<adam::string_hashed> original_paths;
    adam::string_hashed original_default_path;
    if (paths_list)
    {
        for (const auto& [name, param] : paths_list->get_children())
        {
            if (auto* str_param = dynamic_cast<adam::configuration_parameter_string*>(param.get()))
            {
                original_paths.push_back(str_param->get_value());
            }
        }
    }

    std::vector<adam::string_hashed> original_loaded_modules;
    for (const auto& [name, mod] : ctrl.get_registry().modules().get_loaded_modules())
    {
        original_loaded_modules.push_back(name);
    }

    // 2. Clear module cache and unload loaded modules to ensure clean scan environment
    ctrl.get_registry().modules().clear_and_unload_all();

    // 3. Clear module paths list
    if (paths_list)
    {
        if (auto* default_param = dynamic_cast<adam::configuration_parameter_string*>(paths_list->get("0"_ct)))
        {
            original_default_path = default_param->get_value();
            default_param->set_value("");
        }

        while (paths_list->get_children().size() > 1)
        {
            ctrl.get_registry().remove_module_path(1);
        }
    }

    // 4. Add invalid path
    ctrl.get_registry().add_module_path("/invalid_adam_directory_that_does_not_exist_1337"_ct);

    // Should handle non-existent directories gracefully and return false since no changes were made to the module cache
    EXPECT_FALSE(ctrl.get_registry().modules().scan_for_modules());

    // 5. Restore original paths
    ctrl.get_registry().remove_module_path(1); // remove the invalid one
    
    if (paths_list)
    {
        if (auto* default_param = dynamic_cast<adam::configuration_parameter_string*>(paths_list->get("0"_ct)))
        {
            default_param->set_value(original_default_path);
        }
    }

    for (size_t i = 1; i < original_paths.size(); ++i)
    {
        ctrl.get_registry().add_module_path(original_paths[i]);
    }

    // 6. Rescan and reload the originally loaded modules to restore state
    ctrl.get_registry().modules().scan_for_modules();
    for (const auto& name : original_loaded_modules)
    {
        ctrl.get_registry().modules().load_module(name);
    }
}

/** @brief Tests attempting to load a module that has not been scanned or doesn't exist. */
TEST_F(registry_module_manager_test, load_module_invalid)
{
    adam::controller& ctrl = adam::controller::get();
    
    auto invalid_mod_name = "invalid_module"_ct;
    EXPECT_FALSE(ctrl.get_registry().modules().load_module(invalid_mod_name));
    EXPECT_EQ(ctrl.get_registry().modules().get_module(invalid_mod_name.get_hash()), nullptr);
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