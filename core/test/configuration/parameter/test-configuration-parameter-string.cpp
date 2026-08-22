#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, string_any_mode)
{
    configuration_parameter_string param("my_str"_ct, "default"_ct);

    EXPECT_EQ(param.get_type(), configuration_parameter::type_string);
    EXPECT_EQ(param.get_mode(), configuration_parameter_string::value_mode_any);
    EXPECT_EQ(param.get_value(), "default"_ct);
    EXPECT_EQ(param.get_default_value(), "default"_ct);

    EXPECT_TRUE(param.set_value("new_val"_ct));
    EXPECT_EQ(param.get_value(), "new_val"_ct);

    param.reset_to_default();
    EXPECT_EQ(param.get_value(), "default"_ct);
}

TEST(configuration_parameter_test, string_preset_mode)
{
    configuration_parameter_string::presets_container presets;
    presets.emplace("val_a"_ct, std::make_unique<configuration_parameter_string>("val_a"_ct, "val_a"_ct));
    presets.emplace("val_b"_ct, std::make_unique<configuration_parameter_string>("val_b"_ct, "val_b"_ct));

    configuration_parameter_string param("my_str"_ct, "val_a"_ct, std::move(presets));

    EXPECT_EQ(param.get_mode(), configuration_parameter_string::value_mode_preset);
    EXPECT_EQ(param.get_presets().size(), 2u);

    EXPECT_TRUE(param.set_value("val_b"_ct));
    EXPECT_EQ(param.get_value(), "val_b"_ct);

    EXPECT_FALSE(param.set_value("invalid_val"_ct));
    EXPECT_EQ(param.get_value(), "val_b"_ct);

    param.add_preset(std::make_unique<configuration_parameter_string>("val_c"_ct, "val_c"_ct));
    EXPECT_TRUE(param.set_value("val_c"_ct));
    EXPECT_EQ(param.get_value(), "val_c"_ct);
}

TEST(configuration_parameter_test, string_regex_mode)
{
    auto regex_param = std::make_unique<configuration_parameter_string>("pattern"_ct, "^[A-Z]{3}_[0-9]{3}$"_ct);
    configuration_parameter_string param("code"_ct, "ABC_123"_ct, std::move(regex_param));

    EXPECT_EQ(param.get_mode(), configuration_parameter_string::value_mode_regex);

    EXPECT_TRUE(param.set_value("XYZ_999"_ct));
    EXPECT_EQ(param.get_value(), "XYZ_999"_ct);

    EXPECT_FALSE(param.set_value("abc_123"_ct));
    EXPECT_FALSE(param.set_value("ABCD_123"_ct));
    EXPECT_FALSE(param.set_value("ABC_1234"_ct));
    EXPECT_EQ(param.get_value(), "XYZ_999"_ct);
}

TEST(configuration_parameter_test, string_clone)
{
    auto regex_param = std::make_unique<configuration_parameter_string>("pattern"_ct, "^[0-9]+$"_ct);
    configuration_parameter_string original("orig"_ct, "123"_ct, std::move(regex_param));
    original.set_value("456"_ct);

    auto cloned_base = original.clone();
    ASSERT_NE(cloned_base, nullptr);

    auto* cloned = dynamic_cast<configuration_parameter_string*>(cloned_base.get());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->get_type(), configuration_parameter::type_string);
    EXPECT_EQ(cloned->get_mode(), configuration_parameter_string::value_mode_regex);
    EXPECT_EQ(cloned->get_value(), "456"_ct);
    EXPECT_EQ(cloned->get_default_value(), "123"_ct);
    EXPECT_TRUE(cloned->set_value("789"_ct));
    EXPECT_FALSE(cloned->set_value("abc"_ct));
}

TEST(configuration_parameter_test, string_copy_from)
{
    configuration_parameter_string target("target"_ct, "initial"_ct);
    configuration_parameter_string source("source"_ct, "updated"_ct);

    target.copy_from(&source);
    EXPECT_EQ(target.get_value(), "updated"_ct);

    // Null source should be safely ignored
    target.copy_from(nullptr);
    EXPECT_EQ(target.get_value(), "updated"_ct);

    // Incompatible type should be safely ignored
    configuration_parameter_integer int_source("int_src"_ct, 42);
    target.copy_from(&int_source);
    EXPECT_EQ(target.get_value(), "updated"_ct);
}
