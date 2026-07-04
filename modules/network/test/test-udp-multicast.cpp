#include <gtest/gtest.h>
#include "data/port-types/port-udp-multicast.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "data/inspector.hpp"

#if defined(ADAM_PLATFORM_WINDOWS)
#include <winsock2.h>
#endif

#include <thread>
#include <chrono>
#include <cstring>
#include <string>

#if defined(ADAM_PLATFORM_LINUX)
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif

using namespace adam;
using namespace adam::modules::network;

class udp_multicast_test : public ::testing::Test
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

TEST_F(udp_multicast_test, loopback_ipv4)
{
    port_udp_multicast receiver("recv_port");
    port_udp_multicast sender("send_port");

    receiver.set_controller(&controller::get());
    sender.set_controller(&controller::get());

    auto r_params = receiver.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    r_params->get<configuration_parameter_string>("multicast_ip"_ct)->set_value("239.0.0.1"_ct);
    r_params->get<configuration_parameter_string>("local_interface"_ct)->set_value("127.0.0.1"_ct);
    r_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(15021);
    r_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);
    r_params->get<configuration_parameter_integer>("ttl"_ct)->set_value(1);
    r_params->get<configuration_parameter_boolean>("loopback"_ct)->set_value(true);

    auto s_params = sender.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("multicast_ip"_ct)->set_value("239.0.0.1"_ct);
    s_params->get<configuration_parameter_string>("local_interface"_ct)->set_value("127.0.0.1"_ct);
    s_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(0); // ephemeral
    s_params->get<configuration_parameter_integer>("multicast_port"_ct)->set_value(15021);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv4"_ct);
    s_params->get<configuration_parameter_integer>("ttl"_ct)->set_value(1);
    s_params->get<configuration_parameter_boolean>("loopback"_ct)->set_value(true);

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
    const char* test_data = "Hello UDP Multicast IPv4!";
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

#if defined(ADAM_PLATFORM_LINUX)
static std::string get_multicast_capable_ipv6_interface()
{
    struct ifaddrs* ifap = nullptr;
    if (::getifaddrs(&ifap) == -1)
    {
        return "::1";
    }
    std::string ip = "::1";
    for (struct ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6 && (ifa->ifa_flags & IFF_MULTICAST) && (ifa->ifa_flags & IFF_UP))
        {
            char addr_str[INET6_ADDRSTRLEN]{};
            auto* sa = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            if (::inet_ntop(AF_INET6, &sa->sin6_addr, addr_str, sizeof(addr_str)))
            {
                ip = addr_str;
                break;
            }
        }
    }
    ::freeifaddrs(ifap);
    return ip;
}
#endif

TEST_F(udp_multicast_test, loopback_ipv6)
{
    port_udp_multicast receiver("recv_port_v6");
    port_udp_multicast sender("send_port_v6");

    receiver.set_controller(&controller::get());
    sender.set_controller(&controller::get());

    std::string local_interface = "::1";
    #if defined(ADAM_PLATFORM_LINUX)
    local_interface = get_multicast_capable_ipv6_interface();
    #endif

    auto r_params = receiver.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    // Link-local multicast address (all nodes on the link)
    r_params->get<configuration_parameter_string>("multicast_ip"_ct)->set_value("ff02::1"_ct);
    r_params->get<configuration_parameter_string>("local_interface"_ct)->set_value(adam::string_hashed(local_interface.c_str()));
    r_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(15022);
    r_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv6"_ct);
    r_params->get<configuration_parameter_integer>("ttl"_ct)->set_value(1);
    r_params->get<configuration_parameter_boolean>("loopback"_ct)->set_value(true);

    auto s_params = sender.get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
    s_params->get<configuration_parameter_string>("multicast_ip"_ct)->set_value("ff02::1"_ct);
    s_params->get<configuration_parameter_string>("local_interface"_ct)->set_value(adam::string_hashed(local_interface.c_str()));
    s_params->get<configuration_parameter_integer>("local_port"_ct)->set_value(0);
    s_params->get<configuration_parameter_integer>("multicast_port"_ct)->set_value(15022);
    s_params->get<configuration_parameter_string>("ip_version"_ct)->set_value("ipv6"_ct);
    s_params->get<configuration_parameter_integer>("ttl"_ct)->set_value(1);
    s_params->get<configuration_parameter_boolean>("loopback"_ct)->set_value(true);

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
    const char* test_data = "Hello UDP Multicast IPv6!";
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
