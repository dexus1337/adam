#include <gtest/gtest.h>
#include "data/port-types/port-udp-broadcast.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "data/inspector.hpp"

#if defined(ADAM_PLATFORM_WINDOWS)
#include <winsock2.h>
#endif

#include <thread>
#include <chrono>
#include <cstring>

using namespace adam;
using namespace adam::modules::network;

class udp_broadcast_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
#if defined(ADAM_PLATFORM_WINDOWS)
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        buffer_manager::get().initialize();
    }

    void TearDown() override
    {
        buffer_manager::get().destroy();
#if defined(ADAM_PLATFORM_WINDOWS)
        WSACleanup();
#endif
    }
};

TEST_F(udp_broadcast_test, loopback_ipv4)
{
    port_udp_broadcast receiver("recv_port");
    port_udp_broadcast sender("send_port");

    receiver.set_controller(&controller::get());
    sender.set_controller(&controller::get());

    auto r_params = receiver.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("interface"_ct)->set_value("auto"_ct);
    r_params->get<configuration_parameter_integer>("interface_port"_ct)->set_value(15031);

    auto s_params = sender.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("interface"_ct)->set_value("auto"_ct);
    s_params->get<configuration_parameter_integer>("interface_port"_ct)->set_value(0); // ephemeral
    s_params->get<configuration_parameter_string>("broadcast_ip"_ct)->set_value("255.255.255.255"_ct);
    s_params->get<configuration_parameter_integer>("remote_port"_ct)->set_value(15031);

    ASSERT_TRUE(receiver.start());
    ASSERT_TRUE(sender.start());

    auto r_inspector = std::make_shared<data_inspector>();
    r_inspector->create(receiver.get_name());
    receiver.add_inspector(r_inspector);

    std::atomic<bool> read_success{false};
    buffer* recv_buf = nullptr;

    r_inspector->start_inspecting([&](buffer* buf) 
    {
        if (!read_success)
        {
            recv_buf = buf;
            read_success = true;
        }
        recv_buf->release();
    });

    buffer* send_buf = buffer_manager::get().request_buffer(128);
    ASSERT_NE(send_buf, nullptr);
    const char* test_data = "Hello UDP Broadcast!";
    std::memcpy(send_buf->data_as<char>(), test_data, std::strlen(test_data) + 1);
    send_buf->set_size(static_cast<uint32_t>(std::strlen(test_data) + 1));

    ASSERT_TRUE(sender.write(send_buf));
    send_buf->release();

    for (int i = 0; i < 20; ++i)
    {
        if (read_success)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    receiver.remove_inspector(r_inspector);
    r_inspector->destroy();

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);
    recv_buf->release();

    EXPECT_TRUE(sender.stop());
    EXPECT_TRUE(receiver.stop());
}
