#include <gtest/gtest.h>
#include "controller/registry.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/port/port-input-internal.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"

#include <filesystem>
#include <fstream>

namespace adam::test
{
    /** @brief A test wrapper to instantiate registry and access its protected members. */
    class testable_registry : public adam::registry
    {
    public:
        testable_registry() : adam::registry(adam::controller::get()) {}
        
        adam::configuration_parameter_list& get_general() { return m_parameters; }
    };
}

class registry_test : public ::testing::Test
{
protected:
    const std::string test_filepath = "test_adam_registry.bin";
    const std::string adam_config_filepath = "adam-config.bin";

    void SetUp() override
    {
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
    }
};

/** @brief Tests saving and loading a completely empty registry. */
TEST_F(registry_test, empty_save_load)
{
    adam::test::testable_registry reg;
    
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    EXPECT_TRUE(reg.load(test_filepath));
}

/** @brief Tests fully populating the registry, modifying values, and verifying original values are restored upon reload. */
TEST_F(registry_test, save_modify_reload_verify)
{
    adam::test::testable_registry reg;
    
    // 1. Modify the existing default 'language' parameter
    auto* lang_param = static_cast<adam::configuration_parameter_integer*>(reg.get_general().get(adam::string_hashed("language")));
    ASSERT_NE(lang_param, nullptr);
    lang_param->set_value(adam::language_german);
    
    // 2. Create a port
    adam::string_hashed port_name("my_input_port");
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_input_internal::type_name, adam::string_hashed(), &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Change a parameter in the port to verify it restores correctly
    auto* df_param = static_cast<adam::configuration_parameter_string*>(created_port->get_parameters().get(adam::string_hashed("data_format")));
    ASSERT_NE(df_param, nullptr);
    df_param->set_value(adam::string_hashed_ct("json"));

    // Save the populated registry
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    // 3. Modify existing parameters to ensure we load fresh values from file
    lang_param->set_value(adam::language_english);
    df_param->set_value(adam::string_hashed_ct("xml"));

    // 4. Reload from the binary file into a fresh registry to verify persistence
    adam::test::testable_registry loaded_reg;
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    // 5. Verify the values are restored correctly
    auto* loaded_lang_param = loaded_reg.get_general().get(adam::string_hashed("language"));
    ASSERT_NE(loaded_lang_param, nullptr);
    EXPECT_EQ(loaded_lang_param->get_type(), adam::configuration_parameter::integer);
    EXPECT_EQ(static_cast<adam::configuration_parameter_integer*>(loaded_lang_param)->get_value(), adam::language_german);

    // Verify port was recreated and parameters are restored
    EXPECT_EQ(loaded_reg.ports().size(), 1u);
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    EXPECT_EQ(loaded_port->get_type_name(), adam::port_input_internal::type_name);
    
    auto* loaded_df_param = static_cast<adam::configuration_parameter_string*>(loaded_port->get_parameters().get(adam::string_hashed("data_format")));
    ASSERT_NE(loaded_df_param, nullptr);
    EXPECT_EQ(loaded_df_param->get_value(), adam::string_hashed_ct("json"));
}

/** @brief Tests that file I/O operations fail gracefully with bad paths or invalid file formats. */
TEST_F(registry_test, invalid_file_io)
{
    adam::test::testable_registry reg;
    
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
    adam::test::testable_registry reg;

    adam::string_hashed port_name("test_port_1");

    // Attempt to create a port with an invalid type should gracefully fail
    adam::port* invalid_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::string_hashed("non_existent_type"), adam::string_hashed(), &invalid_port), adam::registry::status_error_factory_not_found);
    EXPECT_EQ(invalid_port, nullptr);

    // Create the port using the internal factory
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_input_internal::type_name, adam::string_hashed(), &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);
    EXPECT_EQ(reg.ports().size(), 1u);
    EXPECT_TRUE(reg.ports().contains(port_name));

    // Attempt to create a duplicate port should gracefully fail
    adam::port* duplicate_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_input_internal::type_name, adam::string_hashed(), &duplicate_port), adam::registry::status_error_port_already_exists);
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
    adam::test::testable_registry reg;
    adam::string_hashed port_name("test_port_clear");
    
    EXPECT_EQ(reg.create_port(port_name, adam::port_input_internal::type_name, adam::string_hashed()), adam::registry::status_success);
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
    adam::test::testable_registry reg;
    adam::string_hashed port_name("type_test_port");

    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_input_internal::type_name, adam::string_hashed(), &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Verify type was populated correctly in the constructor
    auto* type_param = static_cast<adam::configuration_parameter_string*>(created_port->get_parameters().get(adam::string_hashed("type")));
    ASSERT_NE(type_param, nullptr);
    EXPECT_EQ(type_param->get_value(), adam::port_input_internal::type_name);

    EXPECT_TRUE(reg.save(test_filepath));

    adam::test::testable_registry loaded_reg;
    EXPECT_TRUE(loaded_reg.load(test_filepath));

    // Verify the port was correctly recreated from the saved type
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    
    auto* loaded_type_param = static_cast<adam::configuration_parameter_string*>(loaded_port->get_parameters().get(adam::string_hashed("type")));
    ASSERT_NE(loaded_type_param, nullptr);
    EXPECT_EQ(loaded_type_param->get_value(), adam::port_input_internal::type_name);
}