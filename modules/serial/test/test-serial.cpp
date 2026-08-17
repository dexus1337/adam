#include <gtest/gtest.h>
#include "data/port-types/port-serial.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

using namespace adam;
using namespace adam::modules::serial;

TEST(port_serial_test, initial_buffer_flush_parameter_definition)
{
    const auto& params = port_serial::get_user_parameters();
    const auto* user_params = params.get<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    const auto* flush_param = user_params->get<configuration_parameter_boolean>("initial_buffer_flush"_ct);
    ASSERT_NE(flush_param, nullptr);
    EXPECT_EQ(flush_param->get_value(), true);

    EXPECT_FALSE(flush_param->get_description(language_english).empty());
    EXPECT_FALSE(flush_param->get_description(language_german).empty());
}

TEST(port_serial_test, timeout_parameters_definition)
{
    const auto& params = port_serial::get_user_parameters();
    const auto* user_params = params.get<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    const auto* read_timeout = user_params->get<configuration_parameter_integer>("read_timeout_ms"_ct);
    ASSERT_NE(read_timeout, nullptr);
    EXPECT_EQ(read_timeout->get_value(), 50);
    EXPECT_FALSE(read_timeout->get_description(language_english).empty());
    EXPECT_FALSE(read_timeout->get_description(language_german).empty());

    const auto* read_interval = user_params->get<configuration_parameter_integer>("read_interval_timeout_ms"_ct);
    ASSERT_NE(read_interval, nullptr);
    EXPECT_EQ(read_interval->get_value(), 5);
    EXPECT_FALSE(read_interval->get_description(language_english).empty());
    EXPECT_FALSE(read_interval->get_description(language_german).empty());

    const auto* write_timeout = user_params->get<configuration_parameter_integer>("write_timeout_ms"_ct);
    ASSERT_NE(write_timeout, nullptr);
    EXPECT_EQ(write_timeout->get_value(), 500);
    EXPECT_FALSE(write_timeout->get_description(language_english).empty());
    EXPECT_FALSE(write_timeout->get_description(language_german).empty());
}

TEST(port_serial_test, initial_buffer_flush_instance_parameter)
{
    port_serial serial_port("test_serial_port"_ct);
    
    auto* user_params = serial_port.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    auto* flush_param = user_params->get<configuration_parameter_boolean>("initial_buffer_flush"_ct);
    ASSERT_NE(flush_param, nullptr);
    EXPECT_EQ(flush_param->get_value(), true);

    flush_param->set_value(false);
    EXPECT_EQ(flush_param->get_value(), false);

    flush_param->set_value(true);
    EXPECT_EQ(flush_param->get_value(), true);
}
