#include <gtest/gtest.h>
#include "commander/module-view.hpp"
#include "types/string-hashed.hpp"

class module_view_test : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/** @brief Tests extraction when the requested module hash is not found in the loaded modules. */
TEST_F(module_view_test, extract_port_type_not_found)
{
    adam::module_view view;
    adam::string_hashed out_type;
    adam::string_hashed out_module;

    view.extract_port_type_and_module(1234, 5678, out_type, out_module);

    EXPECT_EQ(out_type, adam::string_hashed());
    EXPECT_EQ(out_module, adam::string_hashed());
}

/** @brief Tests extraction when the module is found but the pointer is null (simulating a failed load or mock). */
TEST_F(module_view_test, extract_port_type_null_module)
{
    adam::module_view view;
    adam::string_hashed mod_name("my_test_module");
    
    // Manually insert a nullptr for the module
    view.loaded()[mod_name] = nullptr;

    adam::string_hashed out_type;
    adam::string_hashed out_module;

    // Pass the matching module hash, but an arbitrary port type hash
    view.extract_port_type_and_module(1234, mod_name.get_hash(), out_type, out_module);

    // The module name should be extracted successfully
    EXPECT_EQ(out_module, mod_name);
    // The type should remain untouched (empty) because the module pointer was null
    EXPECT_EQ(out_type, adam::string_hashed());
}