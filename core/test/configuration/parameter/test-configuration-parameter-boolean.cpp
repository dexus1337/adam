#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, boolean_basic_operations)
{
    configuration_parameter_boolean param("my_bool"_ct, false);

    EXPECT_EQ(param.get_type(), configuration_parameter::type_boolean);
    EXPECT_FALSE(param.get_value());
    EXPECT_FALSE(param.get_default_value());

    param.set_value(true);
    EXPECT_TRUE(param.get_value());
    EXPECT_TRUE(param.value());

    param.value() = false;
    EXPECT_FALSE(param.get_value());

    param.set_value(true);
    param.reset_to_default();
    EXPECT_FALSE(param.get_value());
}

TEST(configuration_parameter_test, boolean_clone)
{
    configuration_parameter_boolean original("my_bool"_ct, false);
    original.set_value(true);

    auto cloned_base = original.clone();
    ASSERT_NE(cloned_base, nullptr);

    auto* cloned = dynamic_cast<configuration_parameter_boolean*>(cloned_base.get());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->get_type(), configuration_parameter::type_boolean);
    EXPECT_EQ(cloned->get_name(), "my_bool"_ct);
    EXPECT_TRUE(cloned->get_value());
    EXPECT_FALSE(cloned->get_default_value());
}

TEST(configuration_parameter_test, boolean_copy_values_from)
{
    configuration_parameter_boolean target("target"_ct, false);
    configuration_parameter_boolean source("source"_ct, true);

    target.copy_values_from(&source);
    EXPECT_TRUE(target.get_value());

    // Null source should be safely ignored
    target.copy_values_from(nullptr);
    EXPECT_TRUE(target.get_value());

    // Incompatible type should be safely ignored
    configuration_parameter_integer int_source("int_src"_ct, 42);
    target.copy_values_from(&int_source);
    EXPECT_TRUE(target.get_value());
}
