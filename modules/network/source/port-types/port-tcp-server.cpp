#include "data/port-types/port-tcp-server.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-network.hpp"

#include <algorithm>
#include <cstring>

namespace adam::modules::network
{
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

    const configuration_parameter_list& port_tcp_server::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- interface ---
            auto local_iface = create_interface_parameter();
            local_iface->set_description(language_english, "Local interface to listen on (auto = all interfaces)."_ct);
            local_iface->set_description(language_german,  "Lokale Schnittstelle, auf der gelauscht werden soll (auto = alle Schnittstellen)."_ct);
            up->add(std::move(local_iface));

            // --- interface_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("interface_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind and listen on."_ct);
            local_port->set_description(language_german,  "Lokaler Port zum Binden und Lauschen."_ct);
            up->add(std::move(local_port));

            // --- tcp_nodelay ---
            auto nodelay = std::make_unique<adam::configuration_parameter_boolean>("tcp_nodelay"_ct, true);
            nodelay->set_description(language_english, "Apply TCP_NODELAY to each accepted client socket (lower latency)."_ct);
            nodelay->set_description(language_german,  "TCP_NODELAY auf akzeptierten Client-Sockets anwenden (geringere Latenz)."_ct);
            up->add(std::move(nodelay));

            // --- ip_version (ipv4 / ipv6) ---
            configuration_parameter_string::presets_container ip_presets;
            ip_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("ipv4"_ct, "ipv4"_ct));
            ip_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("ipv6"_ct, "ipv6"_ct));
            auto ip_ver = std::make_unique<adam::configuration_parameter_string>("ip_version"_ct, "ipv4"_ct, std::move(ip_presets));
            ip_ver->set_description(language_english, "IP version to use: ipv4 or ipv6."_ct);
            ip_ver->set_description(language_german,  "Zu verwendende IP-Version: ipv4 oder ipv6."_ct);
            up->add(std::move(ip_ver));

            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    // =========================================================================
    // Constructor / Destructor
    // =========================================================================

    port_tcp_server::port_tcp_server(const string_hashed& item_name)
        : port_network(item_name, (sizeof(state_buffer_data) + sizeof(tcp_server_stats) / sizeof(uintptr_t) + 1) * sizeof(uintptr_t))
        , m_listener(static_cast<uintptr_t>(invalid_socket_val))
        , m_clients()
        , m_clients_mutex()
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_tcp_server::get_user_parameters());

        // Cache user-parameter pointers (read-only after construction).
        auto up          = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_interface       = up->get<adam::configuration_parameter_string>("interface"_ct);
        m_interface_port  = up->get<adam::configuration_parameter_integer>("interface_port"_ct);
        m_tcp_nodelay     = up->get<adam::configuration_parameter_boolean>("tcp_nodelay"_ct);
        m_ip_version      = up->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    port_tcp_server::~port_tcp_server()
    {
        stop();
    }

    // =========================================================================
    // start() — create listener socket, bind, listen, start worker thread
    // =========================================================================

    bool port_tcp_server::start()
    {
        set_state(state_starting);
        // --- Resolve the local listen address ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;
        std::string      resolved_ip;

        if (!resolve_bind_address(m_interface->get_value(), "", 0,
                                  static_cast<int>(m_interface_port->get_value()),
                                  m_ip_version->get_value(), SOCK_STREAM,
                                  local_addr, local_addr_len, resolved_ip))
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, socket_error_t::error_address_invalid, "TCP-Server");
            return false;
        }

        // --- Create the TCP listener socket ---
        socket_t sock = ::socket(local_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == invalid_socket_val)
        {
            log_network_socket_error(log::error, log_event::socket_creation_failed, resolve_socket_error(get_last_error()), "TCP-Server");
            return false;
        }

        // --- Reuse address ---
        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        // --- Dual-stack support when ip_version is "auto" ---
        apply_dual_stack_if_auto(sock, local_addr.ss_family, m_ip_version->get_value());

        // --- Bind ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == socket_error_val)
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, resolve_socket_error(get_last_error()), "TCP-Server");
            close_and_clear_socket(sock);
            return false;
        }

        resolve_active_ip(sock);

        // --- Listen ---
        if (::listen(sock, SOMAXCONN) == socket_error_val)
        {
            log_network_socket_error(log::error, log_event::socket_listen_failed, resolve_socket_error(get_last_error()), "TCP-Server");
            close_and_clear_socket(sock);
            return false;
        }

        // --- Non-blocking listener ---
        if (!set_nonblocking(sock, true))
        {
            log_network_message(log::error, log_event::socket_option_failed, "TCP-Server");
            close_and_clear_socket(sock);
            return false;
        }

        m_listener = static_cast<uintptr_t>(sock);

        // Initialize statistics block to zero
        std::memset(&get_state_buffer_data()->user_data<tcp_server_stats>(), 0, sizeof(tcp_server_stats));

        log_network_message(log::info, log_event::tcp_server_started, "TCP-Server",
                            std::format("{} ({}) Port {}", get_active_interface().c_str(), get_active_ip().c_str(), m_active_port));

        return port::start();
    }

    // =========================================================================
    // stop() — close listener and all client sockets
    // =========================================================================

    bool port_tcp_server::stop()
    {
        set_state(state_stopping);
        socket_t listener_sock = static_cast<socket_t>(m_listener);
        if (listener_sock == invalid_socket_val)
        {
            return port::stop();
        }

        m_listener = static_cast<uintptr_t>(invalid_socket_val);
        close_socket(listener_sock);

        std::vector<uintptr_t> clients_to_close;
        {
            adam::spinlock::guard lock(m_clients_mutex);
            clients_to_close = m_clients;
            m_clients.clear();
        }

        for (auto c : clients_to_close)
        {
            socket_t cs = static_cast<socket_t>(c);
            close_socket(cs);
        }

        // Clear statistics
        std::memset(&get_state_buffer_data()->user_data<tcp_server_stats>(), 0, sizeof(tcp_server_stats));

        log_network_message(log::info, log_event::tcp_server_stopped, "TCP-Server");

        m_active_ip.clear();

        return port::stop();
    }

    // =========================================================================
    // read() — accept new clients and receive data from existing ones
    // =========================================================================

    bool port_tcp_server::read(buffer*& buff)
    {
        while (is_running())
        {
            socket_t listener_sock = static_cast<socket_t>(m_listener);
            if (listener_sock == invalid_socket_val) return false;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listener_sock, &read_fds);
            int max_fd = static_cast<int>(listener_sock);

            std::vector<uintptr_t> clients_copy;
            {
                adam::spinlock::guard lock(m_clients_mutex);
                clients_copy = m_clients;
            }

            for (auto c : clients_copy)
            {
                socket_t cs = static_cast<socket_t>(c);
                FD_SET(cs, &read_fds);
                if (static_cast<int>(cs) > max_fd)
                    max_fd = static_cast<int>(cs);
            }

            timeval tv{ 0, 100000 };
            int rv = ::select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);

            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;
                return false;
            }

            if (rv == 0)
            {
                continue;
            }

            // --- 1. Check for new incoming connections ---
            if (FD_ISSET(listener_sock, &read_fds))
            {
                sockaddr_storage client_addr{};
                #if defined(ADAM_PLATFORM_WINDOWS)
                int       client_len = sizeof(client_addr);
                #else
                socklen_t client_len = sizeof(client_addr);
                #endif
                socket_t client_sock = ::accept(listener_sock,
                                                reinterpret_cast<sockaddr*>(&client_addr),
                                                &client_len);

                if (client_sock != invalid_socket_val)
                {
                    set_nonblocking(client_sock, true);

                    if (m_tcp_nodelay->get_value())
                    {
                        #if defined(ADAM_PLATFORM_WINDOWS)
                        BOOL val = TRUE;
                        #else
                        int  val = 1;
                        #endif
                        ::setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY,
                                     reinterpret_cast<const char*>(&val), sizeof(val));
                    }

                    add_client_to_stats(client_sock, client_addr);
                }
                else
                {
                    int err = get_last_error();
                    if (is_running() && static_cast<socket_t>(m_listener) != invalid_socket_val && !is_blocking_error(err))
                    {
                        log_network_socket_error(log::warning, log_event::tcp_server_accept_failed, resolve_socket_error(err), "TCP-Server");
                    }
                }
            }

            // --- 2. Check for readable data on existing client sockets ---
            for (auto c : clients_copy)
            {
                socket_t cs = static_cast<socket_t>(c);
                if (!FD_ISSET(cs, &read_fds)) continue;

                uint8_t temp_buf[4096];
                #if defined(ADAM_PLATFORM_WINDOWS)
                int chunk = ::recv(cs, reinterpret_cast<char*>(temp_buf), sizeof(temp_buf), 0);
                #else
                int chunk = ::recv(cs, temp_buf, sizeof(temp_buf), 0);
                #endif

                if (chunk > 0)
                {
                    buff = buffer_manager::get().request_buffer(chunk);
                    if (buff)
                    {
                        std::memcpy(buff->data_as<uint8_t>(), temp_buf, chunk);
                        buff->set_size(chunk);
                        buff->set_timestamp();
                        return true;
                    }
                }
                else
                {
                    {
                        adam::spinlock::guard lock(m_clients_mutex);
                        auto it = std::find(m_clients.begin(), m_clients.end(), c);
                        if (it != m_clients.end())
                            m_clients.erase(it);

                        remove_client_from_stats(cs);
                    }
                    close_socket(cs);
                }
            }
        }

        return false;
    }

    // =========================================================================
    // write() — broadcast to all connected clients
    // =========================================================================

    bool port_tcp_server::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        std::vector<uintptr_t> clients_copy;
        {
            adam::spinlock::guard lock(m_clients_mutex);
            clients_copy = m_clients;
        }

        if (clients_copy.empty()) return true;

        const char* data = buff->data_as<const char>();
        size_t      size = buff->get_size();

        std::vector<uintptr_t> failed_clients;
        for (auto c : clients_copy)
        {
            socket_t cs = static_cast<socket_t>(c);
            if (!send_all_nonblocking(cs, data, size))
            {
                failed_clients.push_back(c);
            }
        }

        if (!failed_clients.empty())
        {
            {
                adam::spinlock::guard lock(m_clients_mutex);
                for (auto fc : failed_clients)
                {
                    auto it = std::find(m_clients.begin(), m_clients.end(), fc);
                    if (it != m_clients.end())
                        m_clients.erase(it);

                    socket_t fcs = static_cast<socket_t>(fc);
                    remove_client_from_stats(fcs);
                }
            }

            for (auto fc : failed_clients)
            {
                socket_t fcs = static_cast<socket_t>(fc);
                close_socket(fcs);
            }
        }

        return true;
    }

    // =========================================================================
    // Client Tracking & Statistics Helpers (No Duplication)
    // =========================================================================

    void port_tcp_server::add_client_to_stats(socket_t client_sock, const sockaddr_storage& client_addr)
    {
        char client_ip_str[48]{};
        uint32_t client_port = 0;
        if (client_addr.ss_family == AF_INET)
        {
            auto* sa = reinterpret_cast<const sockaddr_in*>(&client_addr);
            ::inet_ntop(AF_INET, &sa->sin_addr, client_ip_str, sizeof(client_ip_str));
            client_port = ntohs(sa->sin_port);
        }
        else if (client_addr.ss_family == AF_INET6)
        {
            auto* sa = reinterpret_cast<const sockaddr_in6*>(&client_addr);
            ::inet_ntop(AF_INET6, &sa->sin6_addr, client_ip_str, sizeof(client_ip_str));
            client_port = ntohs(sa->sin6_port);
        }

        {
            adam::spinlock::guard lock(m_clients_mutex);
            m_clients.push_back(static_cast<uintptr_t>(client_sock));

            tcp_server_stats& stats = get_state_buffer_data()->user_data<tcp_server_stats>();
            for (auto& c_info : stats.clients)
            {
                if (!c_info.active)
                {
                    std::strncpy(c_info.ip, client_ip_str, sizeof(c_info.ip) - 1);
                    c_info.port = client_port;
                    c_info.active = true;
                    stats.active_clients_count++;
                    break;
                }
            }
        }
        log_network_message(log::info, log_event::tcp_server_client_connected, "TCP-Server",
                            std::format("({}:{})", client_ip_str, client_port));
    }

    void port_tcp_server::remove_client_from_stats(socket_t client_sock)
    {
        char client_ip_str[48]{};
        uint32_t client_port = 0;

        sockaddr_storage peer_addr{};
        #if defined(ADAM_PLATFORM_WINDOWS)
        int peer_len = sizeof(peer_addr);
        #else
        socklen_t peer_len = sizeof(peer_addr);
        #endif
        if (::getpeername(client_sock, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len) == 0)
        {
            if (peer_addr.ss_family == AF_INET)
            {
                auto* sa = reinterpret_cast<sockaddr_in*>(&peer_addr);
                ::inet_ntop(AF_INET, &sa->sin_addr, client_ip_str, sizeof(client_ip_str));
                client_port = ntohs(sa->sin_port);
            }
            else if (peer_addr.ss_family == AF_INET6)
            {
                auto* sa = reinterpret_cast<sockaddr_in6*>(&peer_addr);
                ::inet_ntop(AF_INET6, &sa->sin6_addr, client_ip_str, sizeof(client_ip_str));
                client_port = ntohs(sa->sin6_port);
            }

            tcp_server_stats& stats = get_state_buffer_data()->user_data<tcp_server_stats>();
            for (auto& c_info : stats.clients)
            {
                if (c_info.active && std::strcmp(c_info.ip, client_ip_str) == 0 && c_info.port == client_port)
                {
                    c_info.active = false;
                    std::memset(c_info.ip, 0, sizeof(c_info.ip));
                    c_info.port = 0;
                    if (stats.active_clients_count > 0)
                        stats.active_clients_count--;
                    break;
                }
            }
        }
        log_network_message(log::info, log_event::tcp_server_client_disconnected, "TCP-Server",
                            std::format("({}:{})", client_ip_str, client_port));
    }

} // namespace adam::modules::network
