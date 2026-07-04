#include <gtest/gtest.h>
#include "data/port-types/port-tcp-server.hpp"
#include "data/port-types/port-tcp-client.hpp"
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

class tcp_test : public ::testing::Test
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

TEST_F(tcp_test, loopback_ipv4)
{
    port_tcp_server server("server_port");
    port_tcp_client client("client_port");

    server.set_controller(&controller::get());
    client.set_controller(&controller::get());

    auto s_params = server.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("interface"_ct)->set_value("lo"_ct);
    s_params->get<configuration_parameter_integer>("interface_port"_ct)->set_value(15041);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    auto c_params = client.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    c_params->get<configuration_parameter_string>("remote_ip"_ct)->set_value("127.0.0.1"_ct);
    c_params->get<configuration_parameter_integer>("remote_port"_ct)->set_value(15041);
    c_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    ASSERT_TRUE(server.start());
    ASSERT_TRUE(client.start());

    // Give time to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    buffer* send_buf = buffer_manager::get().request_buffer(128);
    ASSERT_NE(send_buf, nullptr);
    const char* test_data = "Hello TCP IPv4!";
    std::memcpy(send_buf->data_as<char>(), test_data, std::strlen(test_data) + 1);
    send_buf->set_size(static_cast<uint32_t>(std::strlen(test_data) + 1));

    auto s_inspector = std::make_shared<data_inspector>();
    s_inspector->create(server.get_name());
    server.add_inspector(s_inspector);

    std::atomic<bool> read_success{false};
    buffer* recv_buf = nullptr;

    s_inspector->start_inspecting([&](buffer* buf) 
    {
        if (!read_success)
        {
            recv_buf = buf;
            read_success = true;
        }
        recv_buf->release();
    });

    EXPECT_TRUE(client.write(send_buf));
    send_buf->release();

    for (int i = 0; i < 20; ++i)
    {
        if (read_success)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    server.remove_inspector(s_inspector);
    s_inspector->destroy();

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);

    auto c_inspector = std::make_shared<data_inspector>();
    c_inspector->create(client.get_name());
    client.add_inspector(c_inspector);

    std::atomic<bool> c_read_success{false};
    buffer* c_recv_buf = nullptr;

    c_inspector->start_inspecting([&](buffer* buf) 
    {
        if (!c_read_success)
        {
            c_recv_buf = buf;
            c_recv_buf->release();
            c_read_success = true;
        }
    });

    // Write back from server to client
    EXPECT_TRUE(server.write(recv_buf));
    recv_buf->release();

    for (int i = 0; i < 20; ++i)
    {
        if (c_read_success)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    client.remove_inspector(c_inspector);
    c_inspector->destroy();

    ASSERT_TRUE(c_read_success);
    ASSERT_NE(c_recv_buf, nullptr);
    EXPECT_STREQ(c_recv_buf->data_as<const char>(), test_data);
    c_recv_buf->release();

    EXPECT_TRUE(client.stop());
    EXPECT_TRUE(server.stop());
}

TEST_F(tcp_test, loopback_ipv6)
{
    port_tcp_server server("server_port_v6");
    port_tcp_client client("client_port_v6");

    server.set_controller(&controller::get());
    client.set_controller(&controller::get());

    auto s_params = server.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("interface"_ct)->set_value("lo"_ct);
    s_params->get<configuration_parameter_integer>("interface_port"_ct)->set_value(15042);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv6"_ct);

    auto c_params = client.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    c_params->get<configuration_parameter_string>("remote_ip"_ct)->set_value("::1"_ct);
    c_params->get<configuration_parameter_integer>("remote_port"_ct)->set_value(15042);
    c_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv6"_ct);

    ASSERT_TRUE(server.start());
    ASSERT_TRUE(client.start());

    // Give time to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    buffer* send_buf = buffer_manager::get().request_buffer(128);
    ASSERT_NE(send_buf, nullptr);
    const char* test_data = "Hello TCP IPv6!";
    std::memcpy(send_buf->data_as<char>(), test_data, std::strlen(test_data) + 1);
    send_buf->set_size(static_cast<uint32_t>(std::strlen(test_data) + 1));

    auto s_inspector = std::make_shared<data_inspector>();
    s_inspector->create(server.get_name());
    server.add_inspector(s_inspector);

    std::atomic<bool> read_success{false};
    buffer* recv_buf = nullptr;

    s_inspector->start_inspecting([&](buffer* buf) {
        if (!read_success)
        {
            recv_buf = buf;
            read_success = true;
        }
        recv_buf->release();
    });

    EXPECT_TRUE(client.write(send_buf));
    send_buf->release();

    for (int i = 0; i < 20; ++i)
    {
        if (read_success)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    server.remove_inspector(s_inspector);
    s_inspector->destroy();

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);

    auto c_inspector = std::make_shared<data_inspector>();
    c_inspector->create(client.get_name());
    client.add_inspector(c_inspector);

    std::atomic<bool> c_read_success{false};
    buffer* c_recv_buf = nullptr;

    c_inspector->start_inspecting([&](buffer* buf) {
        if (!c_read_success)
        {
            c_recv_buf = buf;
            c_recv_buf->release();
            c_read_success = true;
        }
    });

    // Write back from server to client
    EXPECT_TRUE(server.write(recv_buf));
    recv_buf->release();

    for (int i = 0; i < 20; ++i)
    {
        if (c_read_success)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    client.remove_inspector(c_inspector);
    c_inspector->destroy();

    ASSERT_TRUE(c_read_success);
    ASSERT_NE(c_recv_buf, nullptr);
    EXPECT_STREQ(c_recv_buf->data_as<const char>(), test_data);
    c_recv_buf->release();

    EXPECT_TRUE(client.stop());
    EXPECT_TRUE(server.stop());
}
