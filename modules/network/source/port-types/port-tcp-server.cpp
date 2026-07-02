#include "data/port-types/port-tcp-server.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "port-types/socket-helpers.hpp"

#include <algorithm>

namespace adam::modules::network
{
    const configuration_parameter_list& port_tcp_server::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto local_iface_param = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, "0.0.0.0"_ct);
            local_iface_param->set_description(language_english, "The local interface IP to listen on."_ct);
            local_iface_param->set_description(language_german, "Die lokale Schnittstellen-IP zum Lauschen."_ct);
            up->add(std::move(local_iface_param));

            auto local_port_param = std::make_unique<adam::configuration_parameter_integer>("local_port"_ct, 0);
            local_port_param->set_description(language_english, "The local port to listen on."_ct);
            local_port_param->set_description(language_german, "Der lokale Port zum Lauschen."_ct);
            up->add(std::move(local_port_param));

            auto nodelay_param = std::make_unique<adam::configuration_parameter_boolean>("tcp_nodelay"_ct, true);
            nodelay_param->set_description(language_english, "Disable Nagle's algorithm on client connections."_ct);
            nodelay_param->set_description(language_german, "Nagle-Algorithmus bei Client-Verbindungen deaktivieren."_ct);
            up->add(std::move(nodelay_param));

            configuration_parameter_string::presets_container ip_ver_presets = {};
            ip_ver_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("auto"_ct, "auto"_ct));
            ip_ver_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("ipv4"_ct, "ipv4"_ct));
            ip_ver_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("ipv6"_ct, "ipv6"_ct));

            auto ip_ver_param = std::make_unique<adam::configuration_parameter_string>("ip_version"_ct, "auto"_ct, std::move(ip_ver_presets));
            ip_ver_param->set_description(language_english, "The IP version (auto, ipv4, ipv6)."_ct);
            ip_ver_param->set_description(language_german, "Die IP-Version (auto, ipv4, ipv6)."_ct);
            up->add(std::move(ip_ver_param));

            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    port_tcp_server::port_tcp_server(const string_hashed& item_name)
        : port_in_out(item_name),
          m_listener(static_cast<uintptr_t>(INVALID_SOCKET_VAL)),
          m_clients(),
          m_clients_mutex()
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_tcp_server::get_user_parameters());

        auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_local_interface = user_params->get<adam::configuration_parameter_string>("local_interface"_ct);
        m_local_port = user_params->get<adam::configuration_parameter_integer>("local_port"_ct);
        m_tcp_nodelay = user_params->get<adam::configuration_parameter_boolean>("tcp_nodelay"_ct);
        m_ip_version = user_params->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    port_tcp_server::~port_tcp_server()
    {
        stop();
    }

    bool port_tcp_server::start()
    {
        auto* ctrl = get_controller();
        language lang = ctrl ? ctrl->get_language() : language_english;

        sockaddr_storage local_addr;
        int local_addr_len = 0;
        std::string ip_ver = m_ip_version->get_value().c_str();

        if (!resolve_address(m_local_interface->get_value().c_str(), static_cast<int>(m_local_port->get_value()), ip_ver, local_addr, local_addr_len, SOCK_STREAM))
        {
            ctrl->log(log::error, std::format("TCP-Server: {} ({})", get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang)));
            return false;
        }

        socket_t sock = ::socket(local_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("TCP-Server: {} ({})", get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

        #if defined(ADAM_PLATFORM_WINDOWS)
        // Enable dual stack if Auto and IPv6
        if (local_addr.ss_family == AF_INET6 && ip_ver == "auto")
        {
            DWORD ipv6only = 0;
            ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&ipv6only, sizeof(ipv6only));
        }
        #else
        if (local_addr.ss_family == AF_INET6 && ip_ver == "auto")
        {
            int ipv6only = 0;
            ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&ipv6only, sizeof(ipv6only));
        }
        #endif

        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("TCP-Server: {} ({})", get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        if (::listen(sock, SOMAXCONN) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("TCP-Server: {} ({})", get_event_log_text(log_event::socket_listen_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("TCP-Server: {}", get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_listener = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("TCP-Server: {} (Port {})", get_event_log_text(log_event::socket_listen_success, lang), m_local_port->get_value()));

        return port::start();
    }

    bool port_tcp_server::stop()
    {
        socket_t listener_sock = static_cast<socket_t>(m_listener);
        if (listener_sock != INVALID_SOCKET_VAL)
        {
            close_socket(listener_sock);
            m_listener = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
        }

        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (auto c : m_clients)
        {
            socket_t cs = static_cast<socket_t>(c);
            close_socket(cs);
        }
        m_clients.clear();

        auto* ctrl = get_controller();
        ctrl->log(log::info, "TCP-Server: listener and client sockets closed.");

        return port::stop();
    }

    bool port_tcp_server::read(buffer*& buff)
    {
        auto* ctrl = get_controller();
        language lang = ctrl->get_language();

        while (is_started())
        {
            socket_t listener_sock = static_cast<socket_t>(m_listener);
            if (listener_sock == INVALID_SOCKET_VAL) return false;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listener_sock, &read_fds);

            int max_fd = static_cast<int>(listener_sock);

            std::vector<uintptr_t> clients_copy;
            {
                std::lock_guard<std::mutex> lock(m_clients_mutex);
                clients_copy = m_clients;
            }

            for (auto c : clients_copy)
            {
                socket_t cs = static_cast<socket_t>(c);
                FD_SET(cs, &read_fds);
                if (static_cast<int>(cs) > max_fd)
                {
                    max_fd = static_cast<int>(cs);
                }
            }

            timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100 ms

            int rv = ::select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;
                return false;
            }
            else if (rv > 0)
            {
                // 1. Check for new connections
                if (FD_ISSET(listener_sock, &read_fds))
                {
                    sockaddr_storage client_addr;
                    #if defined(ADAM_PLATFORM_WINDOWS)
                    int client_len = sizeof(client_addr);
                    socket_t client_sock = ::accept(listener_sock, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
                    #else
                    socklen_t client_len = sizeof(client_addr);
                    socket_t client_sock = ::accept(listener_sock, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
                    #endif

                    if (client_sock != INVALID_SOCKET_VAL)
                    {
                        set_nonblocking(client_sock, true);

                        // Set TCP_NODELAY
                        if (m_tcp_nodelay->get_value())
                        {
                            #if defined(ADAM_PLATFORM_WINDOWS)
                            BOOL val = TRUE;
                            ::setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
                            #else
                            int val = 1;
                            ::setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
                            #endif
                        }

                        std::lock_guard<std::mutex> lock(m_clients_mutex);
                        m_clients.push_back(static_cast<uintptr_t>(client_sock));
                        ctrl->log(log::info, std::format("TCP-Server: {}", get_event_log_text(log_event::tcp_server_client_connected, lang)));
                    }
                    else
                    {
                        int err = get_last_error();
                        if (!is_blocking_error(err) && ctrl)
                        {
                            socket_error_t err_resolved = resolve_socket_error(err);
                            ctrl->log(log::warning, std::format("TCP-Server: {} ({})", get_event_log_text(log_event::tcp_server_accept_failed, lang), get_error_log_text(err_resolved, lang)));
                        }
                    }
                }

                // 2. Check for readability on client sockets
                for (auto c : clients_copy)
                {
                    socket_t cs = static_cast<socket_t>(c);
                    if (FD_ISSET(cs, &read_fds))
                    {
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
                            // Connection lost or closed
                            std::lock_guard<std::mutex> lock(m_clients_mutex);
                            auto it = std::find(m_clients.begin(), m_clients.end(), c);
                            if (it != m_clients.end())
                            {
                                m_clients.erase(it);
                            }
                            close_socket(cs);
                            ctrl->log(log::info, std::format("TCP-Server: {}", get_event_log_text(log_event::tcp_server_client_disconnected, lang)));
                        }
                    }
                }
            }
        }

        return false;
    }

    bool port_tcp_server::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        std::vector<uintptr_t> clients_copy;
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            clients_copy = m_clients;
        }

        if (clients_copy.empty()) return true; // Sent to all (none connected)

        const char* data = buff->data_as<const char>();
        size_t size = buff->get_size();
        std::vector<uintptr_t> failed_clients;

        for (auto c : clients_copy)
        {
            socket_t cs = static_cast<socket_t>(c);
            #if defined(ADAM_PLATFORM_WINDOWS)
            int sent = ::send(cs, data, static_cast<int>(size), 0);
            #else
            int sent = ::send(cs, data, size, 0);
            #endif

            if (sent < 0)
            {
                int err = get_last_error();
                if (!is_blocking_error(err))
                {
                    failed_clients.push_back(c);
                }
            }
        }

        if (!failed_clients.empty())
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            auto* ctrl = get_controller();
            language lang = ctrl ? ctrl->get_language() : language_english;

            for (auto fc : failed_clients)
            {
                auto it = std::find(m_clients.begin(), m_clients.end(), fc);
                if (it != m_clients.end())
                {
                    m_clients.erase(it);
                }
                socket_t fcs = static_cast<socket_t>(fc);
                close_socket(fcs);
                ctrl->log(log::info, std::format("TCP-Server: {}", get_event_log_text(log_event::tcp_server_client_disconnected, lang)));
            }
        }

        return true;
    }
}
