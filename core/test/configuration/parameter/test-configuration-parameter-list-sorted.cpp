#include <gtest/gtest.h>
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

using namespace adam;
using namespace adam::string_hashed_ct_literals;

TEST(configuration_parameter_test, list_sorted_basic_insert_order_operations)
{
    configuration_parameter_list_sorted sorted_list("my_list"_ct);
    
    // Add parameters
    sorted_list.add(std::make_unique<configuration_parameter_integer>("param_c"_ct, 3));
    sorted_list.add(std::make_unique<configuration_parameter_integer>("param_a"_ct, 1));
    sorted_list.add(std::make_unique<configuration_parameter_integer>("param_b"_ct, 2));
    
    // Verify order is insertion order
    const auto& order = sorted_list.get_order();
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], "param_c"_ct.get_hash());
    EXPECT_EQ(order[1], "param_a"_ct.get_hash());
    EXPECT_EQ(order[2], "param_b"_ct.get_hash());
    
    // Remove parameter
    sorted_list.remove("param_a"_ct.get_hash());
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], "param_c"_ct.get_hash());
    EXPECT_EQ(order[1], "param_b"_ct.get_hash());
    
    // Rename parameter
    sorted_list.rename_child("param_c"_ct.get_hash(), "param_z"_ct);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], "param_z"_ct.get_hash());
    EXPECT_EQ(order[1], "param_b"_ct.get_hash());
    
    // Add param_a back (should append to the end in insert order)
    sorted_list.add(std::make_unique<configuration_parameter_integer>("param_a"_ct, 1));
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], "param_z"_ct.get_hash());
    EXPECT_EQ(order[1], "param_b"_ct.get_hash());
    EXPECT_EQ(order[2], "param_a"_ct.get_hash());
    
    // Clear list
    sorted_list.clear();
    EXPECT_TRUE(order.empty());
}

TEST(configuration_parameter_test, list_sorted_slicing_copy_and_assignment)
{
    // Create source sorted list
    configuration_parameter_list_sorted src("src_list"_ct);
    src.add(std::make_unique<configuration_parameter_integer>("param_c"_ct, 3));
    src.add(std::make_unique<configuration_parameter_integer>("param_a"_ct, 1));
    src.add(std::make_unique<configuration_parameter_integer>("param_b"_ct, 2));
    
    // Upcast to base class reference
    const configuration_parameter_list& base_ref = src;
    
    // Copy construct from base class reference
    configuration_parameter_list_sorted copied(base_ref);
    
    // Verify order is preserved
    const auto& copied_order = copied.get_order();
    ASSERT_EQ(copied_order.size(), 3u);
    EXPECT_EQ(copied_order[0], "param_c"_ct.get_hash());
    EXPECT_EQ(copied_order[1], "param_a"_ct.get_hash());
    EXPECT_EQ(copied_order[2], "param_b"_ct.get_hash());
    
    // Assignment from base class reference
    configuration_parameter_list_sorted assigned("dest_list"_ct);
    assigned = base_ref;
    
    // Verify order is preserved after assignment
    const auto& assigned_order = assigned.get_order();
    ASSERT_EQ(assigned_order.size(), 3u);
    EXPECT_EQ(assigned_order[0], "param_c"_ct.get_hash());
    EXPECT_EQ(assigned_order[1], "param_a"_ct.get_hash());
    EXPECT_EQ(assigned_order[2], "param_b"_ct.get_hash());
}

TEST(configuration_parameter_test, list_sorted_copy_values_from)
{
    configuration_parameter_list_sorted target("sorted_target"_ct);
    target.add(std::make_unique<configuration_parameter_integer>("existing_param"_ct, 100));
    target.add(std::make_unique<configuration_parameter_integer>("retained_param"_ct, 200));

    auto* existing_ptr = target.get<configuration_parameter_integer>("existing_param"_ct);
    auto* retained_ptr = target.get<configuration_parameter_integer>("retained_param"_ct);

    configuration_parameter_list_sorted source("sorted_source"_ct);
    source.add(std::make_unique<configuration_parameter_integer>("existing_param"_ct, 30));
    source.add(std::make_unique<configuration_parameter_integer>("obsolete_param"_ct, 40));

    target.copy_values_from(&source);

    // Order and count should be preserved
    const auto& order = target.get_order();
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], "existing_param"_ct.get_hash());
    EXPECT_EQ(order[1], "retained_param"_ct.get_hash());

    // Pointers must remain stable and values updated in-place
    EXPECT_EQ(target.get<configuration_parameter_integer>("existing_param"_ct), existing_ptr);
    EXPECT_EQ(existing_ptr->get_value(), 30);

    EXPECT_EQ(target.get<configuration_parameter_integer>("retained_param"_ct), retained_ptr);
    EXPECT_EQ(retained_ptr->get_value(), 200);

    // Obsolete parameters are not added
    EXPECT_EQ(target.get("obsolete_param"_ct), nullptr);
}

TEST(configuration_parameter_test, list_sorted_copy_from)
{
    configuration_parameter_list_sorted target("sorted_target"_ct);
    target.add(std::make_unique<configuration_parameter_integer>("old_param"_ct, 999));

    configuration_parameter_list_sorted source("sorted_source"_ct);
    source.add(std::make_unique<configuration_parameter_integer>("item_z"_ct, 30));
    source.add(std::make_unique<configuration_parameter_integer>("item_x"_ct, 10));
    source.add(std::make_unique<configuration_parameter_integer>("item_y"_ct, 20));

    target.copy_from(&source);

    const auto& order = target.get_order();
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], "item_z"_ct.get_hash());
    EXPECT_EQ(order[1], "item_x"_ct.get_hash());
    EXPECT_EQ(order[2], "item_y"_ct.get_hash());

    EXPECT_EQ(target.get("old_param"_ct), nullptr);
    auto* z = target.get<configuration_parameter_integer>("item_z"_ct);
    ASSERT_NE(z, nullptr);
    EXPECT_EQ(z->get_value(), 30);
}
