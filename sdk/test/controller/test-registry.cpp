#include <gtest/gtest.h>
#include "controller/registry.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
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
        testable_registry() : adam::registry(nullptr) {}
        
        adam::configuration_parameter_list& get_general() { return m_general; }
    };
}

class registry_test : public ::testing::Test
{
protected:
    const std::string test_filepath = "test_adam_registry.bin";

    void SetUp() override
    {
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
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

/** @brief Tests fully populating the registry, clearing it, and verifying values upon reload. */
TEST_F(registry_test, save_clear_reload_verify)
{
    adam::test::testable_registry reg;
    
    // 1. Populate general settings
    reg.get_general().add(std::make_unique<adam::configuration_parameter_string>("system_mode", "headless"));
    reg.get_general().add(std::make_unique<adam::configuration_parameter_integer>("max_threads", 16));
    
    // 2. Populate grouped instances
    /*auto ipt_ex = adam::string_hashed("udp_in");
    reg.input_ports().emplace(ipt_ex, std::make_unique<adam::port_input>(ipt_ex));
    
    auto out_ex = adam::string_hashed("tcp_out");
    reg.output_ports().emplace(out_ex, std::make_unique<adam::port_output>(out_ex));
    
    auto flt_ex = adam::string_hashed("noise_filter");
    reg.filters().emplace(flt_ex, std::make_unique<adam::filter>(flt_ex));
    
    auto con_ex = adam::string_hashed("json_conv");
    reg.converters().emplace(con_ex, std::make_unique<adam::converter>(con_ex));*/

    // Save the populated registry
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    // 3. Clear existing parameters to ensure we load fresh from file
    reg.get_general().clear();
    reg.input_ports().clear();
    reg.output_ports().clear();
    reg.filters().clear();
    reg.converters().clear();
    
    EXPECT_EQ(reg.get_general().get(adam::string_hashed("system_mode")), nullptr);
    EXPECT_EQ(reg.get_general().get(adam::string_hashed("max_threads")), nullptr);
    
    // 4. Reload from the binary file
    EXPECT_TRUE(reg.load(test_filepath));
    
    // 5. Verify the values are restored correctly
    auto* mode_param = reg.get_general().get(adam::string_hashed("system_mode"));
    ASSERT_NE(mode_param, nullptr);
    EXPECT_EQ(mode_param->get_type(), adam::configuration_parameter::string);
    EXPECT_EQ(static_cast<adam::configuration_parameter_string*>(mode_param)->get_value(), "headless");

    auto* threads_param = reg.get_general().get(adam::string_hashed("max_threads"));
    ASSERT_NE(threads_param, nullptr);
    EXPECT_EQ(threads_param->get_type(), adam::configuration_parameter::integer);
    EXPECT_EQ(static_cast<adam::configuration_parameter_integer*>(threads_param)->get_value(), 16);
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