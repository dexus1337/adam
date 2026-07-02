#include "data/port-types/port-tcp-client.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    const configuration_parameter_list& port_tcp_client::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto local_iface_param = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, ""_ct);
            local_iface_param->set_description(language_english, "Optional local interface IP to bind to before connecting."_ct);
            local_iface_param->set_description(language_german, "Optionale lokale Schnittstellen-IP zur Bindung vor dem Verbinden."_ct);
            up->add(std::move(local_iface_param));

            auto remote_ip_param = std::make_unique<adam::configuration_parameter_string>("remote_ip"_ct, "127.0.0.1"_ct);
            remote_ip_param->set_description(language_english, "The TCP server IP address."_ct);
            remote_ip_param->set_description(language_german, "Die TCP-Server-IP-Adresse."_ct);
            up->add(std::move(remote_ip_param));

            auto remote_port_param = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port_param->set_description(language_english, "The TCP server port."_ct);
            remote_port_param->set_description(language_german, "Der TCP-Server-Port."_ct);
            up->add(std::move(remote_port_param));

            auto reconnect_param = std::make_unique<adam::configuration_parameter_integer>("reconnect_interval_ms"_ct, 2000);
            reconnect_param->set_description(language_english, "Reconnection retry interval in ms (0 to disable)."_ct);
            reconnect_param->set_description(language_german, "Wiederverbindungs-Intervall in ms (0 zum Deaktivieren)."_ct);
            up->add(std::move(reconnect_param));

            auto nodelay_param = std::make_unique<adam::configuration_parameter_boolean>("tcp_nodelay"_ct, true);
            nodelay_param->set_description(language_english, "Disable Nagle's algorithm (TCP_NODELAY)."_ct);
            nodelay_param->set_description(language_german, "Nagle-Algorithmus deaktivieren (TCP_NODELAY)."_ct);
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

    port_tcp_client::port_tcp_client(const string_hashed& item_name)
        : port_in_out(item_name), m_socket(static_cast<uintptr_t>(INVALID_SOCKET_VAL))
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_tcp_client::get_user_parameters());

        auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_local_interface = user_params->get<adam::configuration_parameter_string>("local_interface"_ct);
        m_remote_ip = user_params->get<adam::configuration_parameter_string>("remote_ip"_ct);
        m_remote_port = user_params->get<adam::configuration_parameter_integer>("remote_port"_ct);
        m_reconnect_interval_ms = user_params->get<adam::configuration_parameter_integer>("reconnect_interval_ms"_ct);
        m_tcp_nodelay = user_params->get<adam::configuration_parameter_boolean>("tcp_nodelay"_ct);
        m_ip_version = user_params->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    port_tcp_client::~port_tcp_client()
    {
        stop();
    }

    bool port_tcp_client::start()
    {
        m_started->set_value(true);
        // Worker thread will handle connect and read
        return port::start();
    }

    bool port_tcp_client::stop()
    {
        std::lock_guard<std::mutex> lock(m_write_mutex);
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock != INVALID_SOCKET_VAL)
        {
            close_socket(sock);
            m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);

            auto* ctrl = get_controller();
            ctrl->log(log::info, "TCP-Client: connection stopped.");
        }
        return port::stop();
    }

    bool port_tcp_client::connect()
    {
        auto* ctrl = get_controller();
        language lang = ctrl->get_language();

        sockaddr_storage dest_addr;
        int dest_addr_len = 0;
        std::string ip_ver = m_ip_version->get_value().c_str();

        if (!resolve_address(m_remote_ip->get_value().c_str(), static_cast<int>(m_remote_port->get_value()), ip_ver, dest_addr, dest_addr_len, SOCK_STREAM))
        {
            ctrl->log(log::error, std::format("TCP-Client: Address resolution failed for {}:{}", m_remote_ip->get_value().c_str(), m_remote_port->get_value()));
            return false;
        }

        socket_t sock = ::socket(dest_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("TCP-Client: {} ({})", get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // Set TCP_NODELAY
        if (m_tcp_nodelay->get_value())
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            BOOL val = TRUE;
            ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
            #else
            int val = 1;
            ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val));
            #endif
        }

        // Optional local interface bind
        if (!m_local_interface->get_value().empty())
        {
            sockaddr_storage local_addr;
            int local_addr_len = 0;
            if (resolve_address(m_local_interface->get_value().c_str(), 0, ip_ver, local_addr, local_addr_len, SOCK_STREAM))
            {
                ::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len);
            }
        }

        // Set to non-blocking for connect timeout / abortability
        if (!set_nonblocking(sock, true))
        {
            close_socket(sock);
            return false;
        }

        int conn_res = ::connect(sock, reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        if (conn_res == SOCKET_ERROR_VAL)
        {
            int err = get_last_error();
            if (!is_blocking_error(err))
            {
                socket_error_t err_resolved = resolve_socket_error(err);
                ctrl->log(log::error, std::format("TCP-Client: {} ({})", get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }

            // Wait with select
            fd_set write_fds, err_fds;
            FD_ZERO(&write_fds);
            FD_ZERO(&err_fds);
            FD_SET(sock, &write_fds);
            FD_SET(sock, &err_fds);

            timeval tv;
            tv.tv_sec = 2; // 2 seconds connect timeout
            tv.tv_usec = 0;

            int select_res = ::select(static_cast<int>(sock) + 1, nullptr, &write_fds, &err_fds, &tv);
            if (select_res <= 0 || FD_ISSET(sock, &err_fds) || !is_started())
            {
                int err_code = get_last_error();
                if (FD_ISSET(sock, &err_fds))
                {
                    #if defined(ADAM_PLATFORM_WINDOWS)
                    int opt_len = sizeof(err_code);
                    ::getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err_code), &opt_len);
                    #else
                    socklen_t opt_len = sizeof(err_code);
                    ::getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err_code), &opt_len);
                    #endif
                }
                socket_error_t err_resolved = resolve_socket_error(err_code == 0 ? WSAECONNREFUSED : err_code);
                ctrl->log(log::error, std::format("TCP-Client: {} ({})", get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }

            int sock_err = 0;
            #if defined(ADAM_PLATFORM_WINDOWS)
            int opt_len = sizeof(sock_err);
            #else
            socklen_t opt_len = sizeof(sock_err);
            #endif
            if (::getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&sock_err), &opt_len) < 0 || sock_err != 0)
            {
                socket_error_t err_resolved = resolve_socket_error(sock_err);
                ctrl->log(log::error, std::format("TCP-Client: {} ({})", get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }
        }

        std::lock_guard<std::mutex> lock(m_write_mutex);
        m_socket = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("TCP-Client: {}", get_event_log_text(log_event::tcp_client_connected, lang)));

        return true;
    }

    bool port_tcp_client::read(buffer*& buff)
    {
        auto* ctrl = get_controller();
        language lang = ctrl->get_language();

        while (is_started())
        {
            socket_t sock = static_cast<socket_t>(m_socket);

            if (sock == INVALID_SOCKET_VAL)
            {
                if (connect())
                {
                    continue;
                }

                // If connection failed, sleep with reconnect interval (abortable)
                int64_t interval = m_reconnect_interval_ms->get_value();
                if (interval <= 0) interval = 2000;

                int64_t slept = 0;
                while (slept < interval && is_started())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    slept += 100;
                }
                return false;
            }

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100 ms

            int rv = ::select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &tv);
            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;

                // socket error
                std::lock_guard<std::mutex> lock(m_write_mutex);
                close_socket(sock);
                m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                return false;
            }
            else if (rv > 0)
            {
                uint8_t temp_buf[4096];
                #if defined(ADAM_PLATFORM_WINDOWS)
                int chunk = ::recv(sock, reinterpret_cast<char*>(temp_buf), sizeof(temp_buf), 0);
                #else
                int chunk = ::recv(sock, temp_buf, sizeof(temp_buf), 0);
                #endif

                if (chunk < 0)
                {
                    int err = get_last_error();
                    if (is_blocking_error(err)) continue;

                    std::lock_guard<std::mutex> lock(m_write_mutex);
                    close_socket(sock);
                    m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                    ctrl->log(log::warning, std::format("TCP-Client: {} (recv failed)", get_event_log_text(log_event::tcp_client_disconnected, lang)));
                    return false;
                }
                else if (chunk == 0)
                {
                    // Server closed connection gracefully
                    std::lock_guard<std::mutex> lock(m_write_mutex);
                    close_socket(sock);
                    m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                    ctrl->log(log::info, std::format("TCP-Client: {}", get_event_log_text(log_event::tcp_client_disconnected, lang)));
                    return false;
                }

                buff = buffer_manager::get().request_buffer(chunk);
                if (!buff) return false;

                std::memcpy(buff->data_as<uint8_t>(), temp_buf, chunk);
                buff->set_size(chunk);
                buff->set_timestamp();
                return true;
            }
        }

        return false;
    }

    bool port_tcp_client::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        std::lock_guard<std::mutex> lock(m_write_mutex);
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        const char* data = buff->data_as<const char>();
        size_t size = buff->get_size();

        #if defined(ADAM_PLATFORM_WINDOWS)
        int sent = ::send(sock, data, static_cast<int>(size), 0);
        #else
        int sent = ::send(sock, data, size, 0);
        #endif

        if (sent < 0)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            auto* ctrl = get_controller();
            language lang = ctrl->get_language();
            ctrl->log(log::warning, std::format("TCP-Client: Send failed ({})", get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }
}
