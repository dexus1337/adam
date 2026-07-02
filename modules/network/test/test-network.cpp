#include <gtest/gtest.h>

#include "data/port-types/port-udp-unicast.hpp"
#include "data/port-types/port-tcp-client.hpp"
#include "data/port-types/port-tcp-server.hpp"
#include "memory/buffer/buffer-manager.hpp"

#if defined(ADAM_PLATFORM_WINDOWS)
#include <winsock2.h>
#endif

#include <thread>
#include <chrono>

using namespace adam;
using namespace adam::modules::network;

class network_test : public ::testing::Test
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

TEST_F(network_test, udp_unicast_loopback_ipv4)
{
    port_udp_unicast receiver("recv_port");
    port_udp_unicast sender("send_port");

    // Configure receiver: bind to 127.0.0.1:15001, ip_version = ipv4
    auto r_params = receiver.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("local_interface"_ct)->set_value("127.0.0.1"_ct);
    r_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(15001);
    r_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    // Configure sender: bind to ephemeral port, send to 127.0.0.1:15001, ip_version = ipv4
    auto s_params = sender.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("local_interface"_ct)->set_value("127.0.0.1"_ct);
    s_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(0);
    s_params->get<configuration_parameter_string>("remote_ip"_ct)->set_value("127.0.0.1"_ct);
    s_params->get<configuration_parameter_integer>("remote_port"_ct)->set_value(15001);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    ASSERT_TRUE(receiver.start());
    ASSERT_TRUE(sender.start());

    buffer* send_buf = buffer_manager::get().request_buffer(128);
    ASSERT_NE(send_buf, nullptr);
    const char* test_data = "Hello UDP IPv4!";
    std::memcpy(send_buf->data_as<char>(), test_data, std::strlen(test_data) + 1);
    send_buf->set_size(static_cast<uint32_t>(std::strlen(test_data) + 1));

    EXPECT_TRUE(sender.write(send_buf));
    send_buf->release();

    buffer* recv_buf = nullptr;
    bool read_success = false;
    for (int i = 0; i < 20; ++i)
    {
        if (receiver.read(recv_buf))
        {
            read_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);
    recv_buf->release();

    EXPECT_TRUE(sender.stop());
    EXPECT_TRUE(receiver.stop());
}

TEST_F(network_test, tcp_client_server_loopback_ipv4)
{
    port_tcp_server server("server_port");
    port_tcp_client client("client_port");

    // Configure server: listen on 127.0.0.1:15002, ip_version = ipv4
    auto s_params = server.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("local_interface"_ct)->set_value("127.0.0.1"_ct);
    s_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(15002);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    // Configure client: connect to 127.0.0.1:15002, ip_version = ipv4
    auto c_params = client.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    c_params->get<configuration_parameter_string>("remote_ip"_ct)->set_value("127.0.0.1"_ct);
    c_params->get<configuration_parameter_integer>("remote_port"_ct)->set_value(15002);
    c_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);

    ASSERT_TRUE(server.start());
    ASSERT_TRUE(client.start());

    // Give some time to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    buffer* send_buf = buffer_manager::get().request_buffer(128);
    ASSERT_NE(send_buf, nullptr);
    const char* test_data = "Hello TCP IPv4!";
    std::memcpy(send_buf->data_as<char>(), test_data, std::strlen(test_data) + 1);
    send_buf->set_size(static_cast<uint32_t>(std::strlen(test_data) + 1));

    EXPECT_TRUE(client.write(send_buf));
    send_buf->release();

    buffer* recv_buf = nullptr;
    bool read_success = false;
    for (int i = 0; i < 20; ++i)
    {
        if (server.read(recv_buf))
        {
            read_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);
    
    // Now write back from server to client
    EXPECT_TRUE(server.write(recv_buf));
    recv_buf->release();

    read_success = false;
    recv_buf = nullptr;
    for (int i = 0; i < 20; ++i)
    {
        if (client.read(recv_buf))
        {
            read_success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    ASSERT_TRUE(read_success);
    ASSERT_NE(recv_buf, nullptr);
    EXPECT_STREQ(recv_buf->data_as<const char>(), test_data);
    recv_buf->release();

    EXPECT_TRUE(client.stop());
    EXPECT_TRUE(server.stop());
}
