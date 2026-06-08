#include <gtest/gtest.h>
#include "controller/registry.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/port/port-internal.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/connection.hpp"
#include "data/format.hpp"
#include "memory/buffer/buffer-manager.hpp"

#include <filesystem>
#include <fstream>

using namespace adam::string_hashed_ct_literals;

namespace adam::test
{
    /** @brief A local controller instance to isolate tests from the global singleton. */
    class local_controller : public adam::controller
    {
    public:
        local_controller() : adam::controller() {}
    };
}

class registry_test : public ::testing::Test
{
protected:
    const std::string test_filepath = "test_adam_registry.bin";
    const std::string adam_config_filepath = "adam-config.bin";

    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
        if (std::filesystem::exists(adam_config_filepath))
            std::filesystem::remove(adam_config_filepath);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
        if (std::filesystem::exists(adam_config_filepath))
            std::filesystem::remove(adam_config_filepath);
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests saving and loading a completely empty registry. */
TEST_F(registry_test, empty_save_load)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    EXPECT_TRUE(reg.load(test_filepath));
}

/** @brief Tests fully populating the registry, modifying values, and verifying original values are restored upon reload. */
TEST_F(registry_test, save_modify_reload_verify)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // 1. Modify the existing default 'language' parameter
    auto* lang_param = static_cast<adam::configuration_parameter_integer*>(reg.get_parameters().get("language"_ct));
    ASSERT_NE(lang_param, nullptr);
    lang_param->set_value(adam::language_german);
    
    // 2. Create a port
    auto port_name = "my_input_port"_ct;
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Change a parameter in the port to verify it restores correctly
    auto* active_param = static_cast<adam::configuration_parameter_boolean*>(created_port->get_parameters().get("started"_ct));
    ASSERT_NE(active_param, nullptr);
    active_param->set_value(true);

    // Save the populated registry
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    // 3. Modify existing parameters to ensure we load fresh values from file
    lang_param->set_value(adam::language_english);
    active_param->set_value(false);

    // 4. Reload from the binary file into a fresh registry to verify persistence
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    // 5. Verify the values are restored correctly
    auto* loaded_lang_param = loaded_reg.get_parameters().get("language"_ct);
    ASSERT_NE(loaded_lang_param, nullptr);
    EXPECT_EQ(loaded_lang_param->get_type(), adam::configuration_parameter::type_integer);
    EXPECT_EQ(static_cast<adam::configuration_parameter_integer*>(loaded_lang_param)->get_value(), adam::language_german);

    // Verify port was recreated and parameters are restored
    EXPECT_EQ(loaded_reg.ports().size(), 1u);
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    EXPECT_EQ(loaded_port->get_type_name(), adam::port_internal::type_name());
    
    auto* loaded_active_param = static_cast<adam::configuration_parameter_boolean*>(loaded_port->get_parameters().get("started"_ct));
    ASSERT_NE(loaded_active_param, nullptr);
    EXPECT_EQ(loaded_active_param->get_value(), true);
}

/** @brief Tests that file I/O operations fail gracefully with bad paths or invalid file formats. */
TEST_F(registry_test, invalid_file_io)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // Saving to an invalid path should fail gracefully
    EXPECT_FALSE(reg.save("/invalid_path_1337/registry.bin"));
    
    // Loading a non-existent file should fail gracefully
    EXPECT_FALSE(reg.load("non_existent_registry.bin"));
    
    // Create a corrupted/invalid binary file
    {
        std::ofstream ofs(test_filepath, std::ios::binary);
        ofs << "This is definitely not a valid ADAM configuration format :)";
        ofs.close();
    }
    
    // Loading an invalid binary file should identify the bad magic number/version and fail
    EXPECT_FALSE(reg.load(test_filepath));
}

/** @brief Tests creating and removing ports via the registry. */
TEST_F(registry_test, create_and_remove_port)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();

    auto port_name = "test_port_1"_ct;

    // Attempt to create a port with an invalid type should gracefully fail
    adam::port* invalid_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, "non_existent_type"_ct, 0, &invalid_port), adam::registry::status_error_factory_not_found);
    EXPECT_EQ(invalid_port, nullptr);

    // Create the port using the internal factory
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);
    EXPECT_EQ(reg.ports().size(), 1u);
    EXPECT_TRUE(reg.ports().contains(port_name));

    // Attempt to create a duplicate port should gracefully fail
    adam::port* duplicate_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &duplicate_port), adam::registry::status_error_port_already_exists);
    EXPECT_EQ(duplicate_port, nullptr);
    EXPECT_EQ(reg.ports().size(), 1u);

    // Remove the port
    EXPECT_EQ(reg.destroy_port(port_name), adam::registry::status_success);
    EXPECT_EQ(reg.ports().size(), 0u);
    EXPECT_FALSE(reg.ports().contains(port_name));
    
    // Attempt to remove a non-existent port should gracefully fail
    EXPECT_EQ(reg.destroy_port(port_name), adam::registry::status_error_port_not_found);
}

/** @brief Tests clearing the registry and verifying state reset. */
TEST_F(registry_test, clear_registry)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto port_name = "test_port_clear"_ct;
    
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0), adam::registry::status_success);
    EXPECT_EQ(reg.ports().size(), 1u);
    
    reg.clear();
    
    EXPECT_TRUE(reg.ports().empty());
    EXPECT_TRUE(reg.filters().empty());
    EXPECT_TRUE(reg.converters().empty());
    EXPECT_TRUE(reg.connections().empty());
}

/** @brief Tests that port properties like type and module_name are persisted. */
TEST_F(registry_test, port_type_and_module_persistence)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto port_name = "type_test_port"_ct;

    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Verify type was populated correctly in the constructor
    auto* type_param = static_cast<adam::configuration_parameter_string*>(created_port->get_parameters().get("type"_ct));
    ASSERT_NE(type_param, nullptr);
    EXPECT_EQ(type_param->get_value(), adam::port_internal::type_name());

    EXPECT_TRUE(reg.save(test_filepath));

    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));

    // Verify the port was correctly recreated from the saved type
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    
    auto* loaded_type_param = static_cast<adam::configuration_parameter_string*>(loaded_port->get_parameters().get("type"_ct));
    ASSERT_NE(loaded_type_param, nullptr);
    EXPECT_EQ(loaded_type_param->get_value(), adam::port_internal::type_name());
}

/** @brief Tests adding and removing module paths in the registry. */
TEST_F(registry_test, add_and_remove_module_paths)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto new_path = "/custom/registry/path"_ct;

    // Add the path
    uint32_t added_idx = 0;
    EXPECT_TRUE(reg.add_module_path(new_path, &added_idx));
    
    // Adding the same path again should fail
    EXPECT_FALSE(reg.add_module_path(new_path));

    // Verify it was added correctly
    auto* paths_list = reg.get_module_paths();
    ASSERT_NE(paths_list, nullptr);
    
    bool found = false;
    for (const auto& [name, param] : paths_list->get_children())
    {
        if (auto* str_param = dynamic_cast<adam::configuration_parameter_string*>(param.get()))
        {
            if (str_param->get_value() == new_path)
            {
                found = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found);

    // Remove the path
    EXPECT_TRUE(reg.remove_module_path(added_idx));
    
    // Removing it again should fail
    EXPECT_FALSE(reg.remove_module_path(added_idx));

    // Verify it was removed (using the internal get method to prove it's gone from the config)
    for (const auto& [name, param] : paths_list->get_children())
    {
        if (auto* str_param = dynamic_cast<adam::configuration_parameter_string*>(param.get()))
        {
            EXPECT_NE(str_param->get_value(), new_path);
        }
    }
}

/** @brief Tests restoring and syncing of unavailable ports. */
TEST_F(registry_test, unavailable_port_retry)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // 1. Manually add an unavailable port to the configuration
    auto upi = std::make_unique<adam::port::unavailable_info>("my_unavail_port"_ct);
    upi->type = "some_missing_type"_ct.get_hash();
    upi->type_module = "missing_module"_ct.get_hash();
    
    auto type_param = std::make_unique<adam::configuration_parameter_string>("type"_ct);
    type_param->set_value("some_missing_type"_ct);
    upi->parameters().add(std::move(type_param));
    
    auto mod_param = std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct);
    mod_param->set_value("missing_module"_ct);
    upi->parameters().add(std::move(mod_param));
    
    reg.unavailable_ports()["my_unavail_port"_ct.get_hash()] = std::move(upi);
    
    // 2. Add it to a connection
    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("my_conn"_ct, &conn), adam::registry::status_success);
    conn->unavailable_inputs().push_back("my_unavail_port"_ct);
    
    if (auto* inputs_list = dynamic_cast<adam::configuration_parameter_list*>(conn->get_parameters().get("inputs"_ct)))
    {
        auto param = std::make_unique<adam::configuration_parameter_reference>("0"_ct);
        param->set_target("my_unavail_port"_ct);
        inputs_list->add(std::move(param));
    }
    
    // 3. Save and reload to ensure it persists
    EXPECT_TRUE(reg.save(test_filepath));
    
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    EXPECT_TRUE(loaded_reg.get_unavailable_ports().contains("my_unavail_port"_ct.get_hash()));
    EXPECT_EQ(loaded_reg.connections().at("my_conn"_ct.get_hash())->unavailable_inputs().size(), 1u);
    EXPECT_EQ(loaded_reg.ports().size(), 0u);
    
    // 4. Now let's pretend the module "missing_module" is loaded.
    // We clear the unavailable ports and insert a valid internal port mock to test the retry mechanism natively
    loaded_reg.unavailable_ports().clear();
    auto upi3 = std::make_unique<adam::port::unavailable_info>("test_retry_port"_ct);
    upi3->type = adam::port_internal::type_name().get_hash();
    upi3->type_module = 0; 
    
    auto type_param3 = std::make_unique<adam::configuration_parameter_string>("type"_ct);
    type_param3->set_value(adam::port_internal::type_name());
    upi3->parameters().add(std::move(type_param3));
    
    loaded_reg.unavailable_ports()["test_retry_port"_ct.get_hash()] = std::move(upi3);
    
    loaded_reg.connections().at("my_conn"_ct.get_hash())->unavailable_inputs()[0] = "test_retry_port"_ct;
    
    loaded_reg.retry_unavailable_ports(0);
    
    // Check if it's now available
    EXPECT_TRUE(loaded_reg.get_unavailable_ports().empty());
    EXPECT_EQ(loaded_reg.ports().size(), 1u);
    EXPECT_TRUE(loaded_reg.ports().contains("test_retry_port"_ct.get_hash()));
    
    auto* loaded_conn = loaded_reg.connections().at("my_conn"_ct.get_hash()).get();
    EXPECT_EQ(loaded_conn->unavailable_inputs().size(), 0u);
    
    loaded_conn->ports_input().iterate([&](const auto& inputs) 
    {
        EXPECT_EQ(inputs.size(), 1u);
        if (!inputs.empty())
        {
            EXPECT_EQ(inputs[0]->get_name(), "test_retry_port"_ct);
        }
    });
}

/** @brief Tests valid_chain calculation on connections based on formats and ports. */
TEST_F(registry_test, connection_valid_chain)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    adam::port* in_port = nullptr;
    adam::port* out_port = nullptr;
    reg.create_port("in_port"_ct, adam::port_internal::type_name(), 0, &in_port);
    reg.create_port("out_port"_ct, adam::port_internal::type_name(), 0, &out_port);
    
    adam::connection* conn = nullptr;
    reg.create_connection("test_valid_conn"_ct, &conn);
    
    // Empty connection -> invalid
    EXPECT_FALSE(conn->is_valid_chain());
    
    reg.connection_add_port("test_valid_conn"_ct.get_hash(), "in_port"_ct.get_hash(), true);
    reg.connection_add_port("test_valid_conn"_ct.get_hash(), "out_port"_ct.get_hash(), false);
    
    // Transparent to transparent -> valid
    EXPECT_TRUE(conn->is_valid_chain());
    
    // Dummy formats
    adam::data_format dummy_fmt1("fmt1"_ct);
    adam::data_format dummy_fmt2("fmt2"_ct);
    
    conn->set_input_format(&dummy_fmt1);
    EXPECT_FALSE(conn->is_valid_chain()); // fmt1 != transparent
    
    conn->set_output_format(&dummy_fmt1);
    EXPECT_TRUE(conn->is_valid_chain()); // fmt1 == fmt1
    
    conn->set_output_format(&dummy_fmt2);
    EXPECT_FALSE(conn->is_valid_chain()); // fmt1 != fmt2
}

/** @brief Tests marking and retrying unavailable connections. */
TEST_F(registry_test, unavailable_connections)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    adam::connection* conn = nullptr;
    reg.create_connection("unavail_conn"_ct, &conn);
    
    // Set dummy module parameter to trigger unavailability when module is marked missing
    if (auto* mod_param = dynamic_cast<adam::configuration_parameter_string*>(conn->get_parameters().get("input_format_module"_ct)))
        mod_param->set_value("my_mod"_ct);
    
    if (auto* in_fmt_param = dynamic_cast<adam::configuration_parameter_string*>(conn->get_parameters().get("input_format"_ct)))
        in_fmt_param->set_value("my_fmt"_ct);

    EXPECT_EQ(reg.connections().size(), 1u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 0u);

    reg.mark_connections_unavailable("my_mod"_ct.get_hash());

    EXPECT_EQ(reg.connections().size(), 0u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 1u);
    
    // Simulate removing the module dependency by falling back to transparent format to safely test retry behavior natively
    auto* uconn = reg.get_unavailable_connections().at("unavail_conn"_ct.get_hash()).get();
    auto* fmt_p = dynamic_cast<adam::configuration_parameter_string*>(uconn->get_parameters().get("input_format"_ct));
    fmt_p->set_value(""_ct);
    
    auto* fmt_p_mod = dynamic_cast<adam::configuration_parameter_string*>(uconn->get_parameters().get("input_format_module"_ct));
    fmt_p_mod->set_value(""_ct);
    
    reg.retry_unavailable_connections("my_mod"_ct.get_hash());
    
    EXPECT_EQ(reg.connections().size(), 1u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 0u);
}