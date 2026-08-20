#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, integer_any_mode)
{
    configuration_parameter_integer param("my_int"_ct, 10);

    EXPECT_EQ(param.get_type(), configuration_parameter::type_integer);
    EXPECT_EQ(param.get_mode(), configuration_parameter_integer::value_mode_any);
    EXPECT_EQ(param.get_value(), 10);
    EXPECT_EQ(param.get_default_value(), 10);
    EXPECT_EQ(param.get_value_as<int>(), 10);

    EXPECT_TRUE(param.set_value(42));
    EXPECT_EQ(param.get_value(), 42);

    param.reset_to_default();
    EXPECT_EQ(param.get_value(), 10);
}

TEST(configuration_parameter_test, integer_range_mode)
{
    configuration_parameter_integer param("my_int"_ct, 5, 0, 10);

    EXPECT_EQ(param.get_mode(), configuration_parameter_integer::value_mode_range);
    EXPECT_EQ(param.get_min_value(), 0);
    EXPECT_EQ(param.get_max_value(), 10);

    EXPECT_TRUE(param.set_value(8));
    EXPECT_EQ(param.get_value(), 8);

    EXPECT_FALSE(param.set_value(-1));
    EXPECT_EQ(param.get_value(), 8);

    EXPECT_FALSE(param.set_value(11));
    EXPECT_EQ(param.get_value(), 8);

    param.set_range(-50, 50);
    EXPECT_TRUE(param.set_value(-25));
    EXPECT_EQ(param.get_value(), -25);
}

TEST(configuration_parameter_test, integer_preset_mode)
{
    configuration_parameter_integer::presets_container presets = {10, 20, 30};
    configuration_parameter_integer param("my_int"_ct, 20, presets);

    EXPECT_EQ(param.get_mode(), configuration_parameter_integer::value_mode_preset);
    EXPECT_EQ(param.get_presets().size(), 3u);

    EXPECT_TRUE(param.set_value(30));
    EXPECT_EQ(param.get_value(), 30);

    EXPECT_FALSE(param.set_value(25));
    EXPECT_EQ(param.get_value(), 30);

    param.add_preset(50);
    EXPECT_TRUE(param.set_value(50));
    EXPECT_EQ(param.get_value(), 50);
}

TEST(configuration_parameter_test, integer_clone)
{
    configuration_parameter_integer::presets_container presets = {10, 20, 30};
    configuration_parameter_integer original("orig"_ct, 20, presets);
    original.set_value(30);

    auto cloned_base = original.clone();
    ASSERT_NE(cloned_base, nullptr);

    auto* cloned = dynamic_cast<configuration_parameter_integer*>(cloned_base.get());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->get_type(), configuration_parameter::type_integer);
    EXPECT_EQ(cloned->get_mode(), configuration_parameter_integer::value_mode_preset);
    EXPECT_EQ(cloned->get_value(), 30);
    EXPECT_EQ(cloned->get_default_value(), 20);
    EXPECT_EQ(cloned->get_presets().size(), 3u);
}

TEST(configuration_parameter_test, integer_copy_from)
{
    configuration_parameter_integer target("target"_ct, 10);
    configuration_parameter_integer source("source"_ct, 999);

    target.copy_from(&source);
    EXPECT_EQ(target.get_value(), 999);

    // Null source should be safely ignored
    target.copy_from(nullptr);
    EXPECT_EQ(target.get_value(), 999);

    // Incompatible type should be safely ignored
    configuration_parameter_string str_source("str_src"_ct, "text"_ct);
    target.copy_from(&str_source);
    EXPECT_EQ(target.get_value(), 999);
}
