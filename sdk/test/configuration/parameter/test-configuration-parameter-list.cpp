#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, list_basic_operations)
{
    configuration_parameter_list list("my_list"_ct);

    EXPECT_EQ(list.get_type(), configuration_parameter::type_list);
    EXPECT_TRUE(list.get_children().empty());

    list.add(std::make_unique<configuration_parameter_integer>("int_child"_ct, 42));
    list.add(std::make_unique<configuration_parameter_string>("str_child"_ct, "hello"_ct));

    EXPECT_EQ(list.get_children().size(), 2u);
    EXPECT_NE(list.get("int_child"_ct), nullptr);
    EXPECT_NE(list.get("str_child"_ct), nullptr);

    auto* typed_int = list.get<configuration_parameter_integer>("int_child"_ct);
    ASSERT_NE(typed_int, nullptr);
    EXPECT_EQ(typed_int->get_value(), 42);

    // Rename child
    EXPECT_TRUE(list.rename_child("str_child"_ct.get_hash(), "renamed_str"_ct));
    EXPECT_EQ(list.get("str_child"_ct), nullptr);
    EXPECT_NE(list.get("renamed_str"_ct), nullptr);

    // Remove child
    list.remove("int_child"_ct.get_hash());
    EXPECT_EQ(list.get("int_child"_ct), nullptr);
    EXPECT_EQ(list.get_children().size(), 1u);

    list.clear();
    EXPECT_TRUE(list.get_children().empty());
}

TEST(configuration_parameter_test, list_copy_and_assignment)
{
    configuration_parameter_list original("orig"_ct);
    original.add(std::make_unique<configuration_parameter_integer>("child_a"_ct, 10));
    original.add(std::make_unique<configuration_parameter_string>("child_b"_ct, "test"_ct));

    // Copy construct
    configuration_parameter_list copied(original);
    EXPECT_EQ(copied.get_children().size(), 2u);
    auto* a = copied.get<configuration_parameter_integer>("child_a"_ct);
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a->get_value(), 10);

    // Modify original, copied should remain unchanged (deep copy)
    original.get<configuration_parameter_integer>("child_a"_ct)->set_value(99);
    EXPECT_EQ(a->get_value(), 10);

    // Assignment
    configuration_parameter_list assigned("assigned"_ct);
    assigned = original;
    EXPECT_EQ(assigned.get_children().size(), 2u);
    EXPECT_EQ(assigned.get<configuration_parameter_integer>("child_a"_ct)->get_value(), 99);
}

TEST(configuration_parameter_test, list_copy_from_backward_compatibility)
{
    // Target represents newly loaded class schema with default parameters
    configuration_parameter_list target("root"_ct);
    target.add(std::make_unique<configuration_parameter_string>("existing_param"_ct, "default_val"_ct));
    target.add(std::make_unique<configuration_parameter_integer>("new_param"_ct, 100)); // Missing in older config

    // Source represents an older saved configuration file
    configuration_parameter_list source("root"_ct);
    source.add(std::make_unique<configuration_parameter_string>("existing_param"_ct, "loaded_val"_ct));
    source.add(std::make_unique<configuration_parameter_double>("obsolete_param"_ct, 123.45)); // Removed in new schema

    target.copy_from(&source);

    // 1. Existing parameter value is updated from file
    auto* existing = target.get<configuration_parameter_string>("existing_param"_ct);
    ASSERT_NE(existing, nullptr);
    EXPECT_EQ(existing->get_value(), "loaded_val"_ct);

    // 2. New parameter retains its default value
    auto* new_param = target.get<configuration_parameter_integer>("new_param"_ct);
    ASSERT_NE(new_param, nullptr);
    EXPECT_EQ(new_param->get_value(), 100);

    // 3. Obsolete parameter from file is not added to the target
    EXPECT_EQ(target.get("obsolete_param"_ct), nullptr);
}

TEST(configuration_parameter_test, list_copy_from_empty_target)
{
    configuration_parameter_list target("root"_ct);

    configuration_parameter_list source("root"_ct);
    source.add(std::make_unique<configuration_parameter_integer>("a"_ct, 1));
    source.add(std::make_unique<configuration_parameter_string>("b"_ct, "hello"_ct));

    target.copy_from(&source);

    EXPECT_EQ(target.get_children().size(), 2u);
    auto* a = target.get<configuration_parameter_integer>("a"_ct);
    auto* b = target.get<configuration_parameter_string>("b"_ct);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(a->get_value(), 1);
    EXPECT_EQ(b->get_value(), "hello"_ct);
}
