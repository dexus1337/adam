#pragma once

/**
 * @file    port-network.hpp
 * @author  dexus1337
 * @brief   Defines the base class for all network ports (TCP, UDP), including socket types,
 *          semantic error mappings, bilingual logging tables, and address resolution helpers.
 * @version 3.0
 * @date    05.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>
#include <string>
#include <string_view>
#include <atomic>
#include <vector>
#include <memory>
#include <functional>

#if defined(ADAM_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL   SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    using socket_t = int;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL   (-1)
#endif

namespace adam::modules::network
{
    /**
     * @enum  socket_error_t
     * @brief Semantic error categories mapped from platform-specific error codes.
     */
    enum class socket_error_t
    {
        success = 0,
        error_address_invalid,
        error_host_unreachable,
        error_connection_refused,
        error_connection_timeout,
        error_connection_lost,
        error_address_in_use,
        error_permission_denied,
        error_network_down,
        error_unknown
    };

    /**
     * @enum  log_event
     * @brief Log events shared across network ports.
     */
    enum class log_event
    {
        socket_created,
        socket_creation_failed,
        socket_bind_success,
        socket_bind_failed,
        socket_listen_success,
        socket_listen_failed,
        socket_connect_success,
        socket_connect_failed,
        socket_option_failed,
        udp_socket_closed,
        udp_send_failed,
        multicast_join_success,
        multicast_join_failed,
        multicast_address_invalid,
        tcp_client_connected,
        tcp_client_disconnected,
        tcp_client_stopped,
        tcp_client_send_failed,
        tcp_address_resolution_failed,
        tcp_server_started,
        tcp_server_stopped,
        tcp_server_client_connected,
        tcp_server_client_disconnected,
        tcp_server_accept_failed,
        tcp_server_send_failed
    };

    struct adapter_info
    {
        std::string friendly_name;
        std::string adapter_name;
        std::vector<std::string> ipv4_addresses;
        std::vector<std::string> ipv6_addresses;
        unsigned int index = 0;
    };

    class ADAM_NETWORK_API port_network : public port_in_out
    {
    public:
        explicit port_network(const string_hashed& item_name, uint32_t state_buffer_size = (sizeof(state_buffer_data) / sizeof(uintptr_t) + 1) * sizeof(uintptr_t));
        virtual ~port_network() = default;

        std::string get_active_interface() const;
        const std::string& get_active_ip() const { return m_active_ip; }

    protected:
        void resolve_active_ip(uintptr_t socket_handle);

        // Logging Helpers (Bilingual)
        void log_network_message(log::level lvl, log_event ev, std::string_view prefix = "", const std::string& extra = "");
        void log_network_socket_error(log::level lvl, log_event ev, socket_error_t err, std::string_view prefix = "");

        static std::string_view get_error_log_text(socket_error_t err, adam::language lang);
        static std::string_view get_event_log_text(log_event ev, adam::language lang);

        // Error code translation
        static int get_last_error();
        static void close_socket(socket_t& s);
        static bool set_nonblocking(socket_t s, bool non_blocking);
        static bool is_blocking_error(int error);
        static socket_error_t resolve_socket_error(int os_error);

        // Address resolution helpers
        static bool resolve_address(const adam::string_hashed& host, int port,
                                    const adam::string_hashed& ip_version,
                                    sockaddr_storage& out_addr, int& out_addr_len, int sock_type);
        static void apply_dual_stack_if_auto(socket_t sock, int family,
                                             const adam::string_hashed& ip_ver);

        // Local Interface Management
        static std::vector<adapter_info>& get_adapter_cache();
        static void refresh_adapter_cache();
        static std::vector<std::string> get_system_interfaces();
        static std::string resolve_auto_interface_for_remote(const adam::string_hashed& remote_ip, int remote_port, const adam::string_hashed& ip_version);
        static std::string resolve_interface_to_ip(const adam::string_hashed& interface_name, int family);

        static bool resolve_local_interface_to_ip(const adam::string_hashed& interface_val,
                                                  const adam::string_hashed& remote_ip,
                                                  int remote_port,
                                                  const adam::string_hashed& ip_version,
                                                  std::string& resolved_ip);
        static unsigned int get_interface_index(const adam::string_hashed& interface_val,
                                                const adam::string_hashed& dest_ip,
                                                int dest_port,
                                                const adam::string_hashed& ip_version);
        static std::unique_ptr<adam::configuration_parameter_string> create_interface_parameter();
        static std::unique_ptr<adam::configuration_parameter_string> create_ip_regex_parameter();

        // Consolidated Address Bind Helper
        static bool resolve_bind_address(const adam::string_hashed& interface_val,
                                         const adam::string_hashed& remote_ip,
                                         int remote_port,
                                         int local_port,
                                         const adam::string_hashed& ip_version,
                                         int sock_type,
                                         sockaddr_storage& out_addr,
                                         int& out_addr_len,
                                         std::string& resolved_ip);

        // Robust Send Helpers
        bool send_all(socket_t sock, const char* data, size_t size);
        bool send_all_nonblocking(socket_t sock, const char* data, size_t size);

        std::string m_active_ip;
        std::string m_active_interface;
    };
}
