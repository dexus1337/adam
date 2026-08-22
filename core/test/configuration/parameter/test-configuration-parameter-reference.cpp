#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, reference_basic_operations)
{
    configuration_parameter_reference param("my_ref"_ct, "default_target"_ct);

    EXPECT_EQ(param.get_type(), configuration_parameter::type_reference);
    EXPECT_EQ(param.get_target(), "default_target"_ct);
    EXPECT_EQ(param.get_default_target(), "default_target"_ct);

    param.set_target("new_target"_ct);
    EXPECT_EQ(param.get_target(), "new_target"_ct);

    param.reset_to_default();
    EXPECT_EQ(param.get_target(), "default_target"_ct);
}

TEST(configuration_parameter_test, reference_clone)
{
    configuration_parameter_reference original("my_ref"_ct, "default_target"_ct);
    original.set_target("custom_target"_ct);

    auto cloned_base = original.clone();
    ASSERT_NE(cloned_base, nullptr);

    auto* cloned = dynamic_cast<configuration_parameter_reference*>(cloned_base.get());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->get_type(), configuration_parameter::type_reference);
    EXPECT_EQ(cloned->get_target(), "custom_target"_ct);
    EXPECT_EQ(cloned->get_default_target(), "default_target"_ct);
}

TEST(configuration_parameter_test, reference_copy_values_from)
{
    configuration_parameter_reference target("target"_ct, "init_target"_ct);
    configuration_parameter_reference source("source"_ct, "source_target"_ct);

    target.copy_values_from(&source);
    EXPECT_EQ(target.get_target(), "source_target"_ct);

    // Null source should be safely ignored
    target.copy_values_from(nullptr);
    EXPECT_EQ(target.get_target(), "source_target"_ct);

    // Incompatible type should be safely ignored
    configuration_parameter_string str_source("str_src"_ct, "text"_ct);
    target.copy_values_from(&str_source);
    EXPECT_EQ(target.get_target(), "source_target"_ct);
}
