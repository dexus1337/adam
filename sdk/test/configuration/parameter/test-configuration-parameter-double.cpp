#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-double.hpp"

using namespace adam;

TEST(configuration_parameter_double_test, any_mode)
{
    configuration_parameter_double param("my_double", 1.5);
    
    EXPECT_EQ(param.get_mode(), configuration_parameter_double::value_mode_any);
    EXPECT_DOUBLE_EQ(param.get_value(), 1.5);
    EXPECT_DOUBLE_EQ(param.get_default_value(), 1.5);
    
    EXPECT_TRUE(param.set_value(3.14));
    EXPECT_DOUBLE_EQ(param.get_value(), 3.14);
    
    param.reset_to_default();
    EXPECT_DOUBLE_EQ(param.get_value(), 1.5);
}

TEST(configuration_parameter_double_test, range_mode)
{
    configuration_parameter_double param("my_double", 1.5, 0.0, 5.0);
    
    EXPECT_EQ(param.get_mode(), configuration_parameter_double::value_mode_range);
    EXPECT_DOUBLE_EQ(param.get_min_value(), 0.0);
    EXPECT_DOUBLE_EQ(param.get_max_value(), 5.0);
    
    EXPECT_TRUE(param.set_value(4.99));
    EXPECT_DOUBLE_EQ(param.get_value(), 4.99);
    
    EXPECT_FALSE(param.set_value(-0.1));
    EXPECT_DOUBLE_EQ(param.get_value(), 4.99); // Value unchanged
    
    EXPECT_FALSE(param.set_value(5.01));
    EXPECT_DOUBLE_EQ(param.get_value(), 4.99); // Value unchanged
    
    param.set_range(-10.0, 10.0);
    EXPECT_TRUE(param.set_value(-5.0));
    EXPECT_DOUBLE_EQ(param.get_value(), -5.0);
}

TEST(configuration_parameter_double_test, preset_mode)
{
    configuration_parameter_double::presets_container presets = {1.0, 2.0, 3.5};
    configuration_parameter_double param("my_double", 2.0, presets);
    
    EXPECT_EQ(param.get_mode(), configuration_parameter_double::value_mode_preset);
    EXPECT_EQ(param.get_presets().size(), 3u);
    
    EXPECT_TRUE(param.set_value(3.5));
    EXPECT_DOUBLE_EQ(param.get_value(), 3.5);
    
    EXPECT_FALSE(param.set_value(2.5));
    EXPECT_DOUBLE_EQ(param.get_value(), 3.5); // Value unchanged
    
    param.add_preset(5.5);
    EXPECT_TRUE(param.set_value(5.5));
    EXPECT_DOUBLE_EQ(param.get_value(), 5.5);
}

TEST(configuration_parameter_double_test, clone)
{
    configuration_parameter_double::presets_container presets = {1.0, 2.0, 3.5};
    configuration_parameter_double original("orig", 2.0, presets);
    original.set_value(3.5);
    
    auto cloned_base = original.clone();
    ASSERT_NE(cloned_base, nullptr);
    
    auto* cloned = dynamic_cast<configuration_parameter_double*>(cloned_base.get());
    ASSERT_NE(cloned, nullptr);
    
    EXPECT_EQ(cloned->get_mode(), configuration_parameter_double::value_mode_preset);
    EXPECT_DOUBLE_EQ(cloned->get_value(), 3.5);
    EXPECT_DOUBLE_EQ(cloned->get_default_value(), 2.0);
    EXPECT_EQ(cloned->get_presets().size(), 3u);
}
