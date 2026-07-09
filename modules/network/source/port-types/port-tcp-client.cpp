#include "data/port-types/port-tcp-client.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-network.hpp"

namespace adam::modules::network
{

    const configuration_parameter_list& port_tcp_client::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- interface ---
            auto local_iface = create_interface_parameter();
            local_iface->set_description(language_english, "Local interface to bind before connecting (auto = automatic)."_ct);
            local_iface->set_description(language_german,  "Lokale Schnittstelle vor dem Verbinden (auto = automatisch)."_ct);
            up->add(std::move(local_iface));

            // --- interface_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("interface_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind to (0 = any)."_ct);
            local_port->set_description(language_german,  "Lokaler Port zur Bindung (0 = beliebig)."_ct);
            up->add(std::move(local_port));

            // --- remote_ip ---
            auto remote_ip = std::make_unique<adam::configuration_parameter_string>("remote_ip"_ct, "127.0.0.1"_ct, create_ip_regex_parameter());
            remote_ip->set_description(language_english, "Remote server IP address or hostname."_ct);
            remote_ip->set_description(language_german,  "IP-Adresse oder Hostname des entfernten Servers."_ct);
            up->add(std::move(remote_ip));

            // --- remote_port ---
            auto remote_port = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port->set_description(language_english, "Remote server port."_ct);
            remote_port->set_description(language_german,  "Port des entfernten Servers."_ct);
            up->add(std::move(remote_port));

            // --- reconnect_interval_ms ---
            auto reconnect = std::make_unique<adam::configuration_parameter_integer>("reconnect_interval_ms"_ct, 2000);
            reconnect->set_description(language_english, "Milliseconds between reconnect attempts (0 = disabled)."_ct);
            reconnect->set_description(language_german,  "Millisekunden zwischen Wiederverbindungsversuchen (0 = deaktiviert)."_ct);
            up->add(std::move(reconnect));

            // --- tcp_nodelay ---
            auto nodelay = std::make_unique<adam::configuration_parameter_boolean>("tcp_nodelay"_ct, true);
            nodelay->set_description(language_english, "Disable Nagle's algorithm (TCP_NODELAY) for lower latency."_ct);
            nodelay->set_description(language_german,  "Nagle-Algorithmus deaktivieren (TCP_NODELAY) für geringere Latenz."_ct);
            up->add(std::move(nodelay));

            // --- ip_version (auto / ipv4 / ipv6) ---
            configuration_parameter_string::presets_container ip_presets;
            ip_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("auto"_ct, "auto"_ct));
            ip_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("ipv4"_ct, "ipv4"_ct));
            ip_presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("ipv6"_ct, "ipv6"_ct));
            auto ip_ver = std::make_unique<adam::configuration_parameter_string>("ip_version"_ct, "auto"_ct, std::move(ip_presets));
            ip_ver->set_description(language_english, "IP version to use: auto, ipv4, or ipv6."_ct);
            ip_ver->set_description(language_german,  "Zu verwendende IP-Version: auto, ipv4 oder ipv6."_ct);
            up->add(std::move(ip_ver));

            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    port_tcp_client::port_tcp_client(const string_hashed& item_name)
        : port_network(item_name)
        , m_socket(static_cast<uintptr_t>(invalid_socket_val))
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_tcp_client::get_user_parameters());

        // Cache user-parameter pointers (read-only after construction).
        auto up                 = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_interface             = up->get<adam::configuration_parameter_string>("interface"_ct);
        m_interface_port        = up->get<adam::configuration_parameter_integer>("interface_port"_ct);
        m_remote_ip             = up->get<adam::configuration_parameter_string>("remote_ip"_ct);
        m_remote_port           = up->get<adam::configuration_parameter_integer>("remote_port"_ct);
        m_reconnect_interval_ms = up->get<adam::configuration_parameter_integer>("reconnect_interval_ms"_ct);
        m_tcp_nodelay           = up->get<adam::configuration_parameter_boolean>("tcp_nodelay"_ct);
        m_ip_version            = up->get<adam::configuration_parameter_string>("ip_version"_ct);

        m_use_spinlock_for_write = true;
    }

    port_tcp_client::~port_tcp_client()
    {
        stop();
    }

    bool port_tcp_client::start()
    {
        set_state(state_starting);
        return port::start();
    }

    bool port_tcp_client::stop()
    {
        set_state(state_stopping);
        m_active_ip.clear();

        socket_t sock = static_cast<socket_t>(invalid_socket_val);
        {
            spinlock::guard lock(m_spinlock);
            sock     = static_cast<socket_t>(m_socket);
            m_socket = static_cast<uintptr_t>(invalid_socket_val);
        }

        if (sock == invalid_socket_val) return port::stop();

        close_socket(sock);
        log_network_message(log::info, log_event::tcp_client_stopped, "TCP-Client");

        return port::stop();
    }

    void port_tcp_client::reset_socket_with_log(adam::log::level lvl, bool should_log)
    {
        socket_t sock = static_cast<socket_t>(invalid_socket_val);
        {
            spinlock::guard lock(m_spinlock);
            sock     = static_cast<socket_t>(m_socket);
            m_socket = static_cast<uintptr_t>(invalid_socket_val);
        }

        m_active_ip.clear();

        if (sock == invalid_socket_val) return;

        close_socket(sock);

        if (should_log && is_running())
        {
            log_network_message(lvl, log_event::tcp_client_disconnected, "TCP-Client");
        }
    }

    bool port_tcp_client::connect()
    {
        if (!is_running()) return false;

        // --- Resolve the remote server address ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_remote_ip->get_value(), static_cast<int>(m_remote_port->get_value()), m_ip_version->get_value(), dest_addr, dest_addr_len, SOCK_STREAM))
        {
            log_network_message(log::error, log_event::tcp_address_resolution_failed, "TCP-Client", std::format("({}:{})", m_remote_ip->get_value().c_str(), m_remote_port->get_value()));
            return false;
        }

        // --- Create the TCP socket ---
        socket_t sock = ::socket(dest_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == invalid_socket_val)
        {
            log_network_socket_error(log::error, log_event::socket_creation_failed, resolve_socket_error(get_last_error()), "TCP-Client");
            set_state(state_error);
            return false;
        }

        // --- TCP_NODELAY ---
        if (m_tcp_nodelay->get_value())
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            BOOL val = TRUE;
            #else
            int  val = 1;
            #endif
            ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&val), sizeof(val));
        }

        // --- Local interface bind ---
        std::string resolved_ip;
        if (!resolve_local_interface_to_ip
        (
            m_interface->get_value(),
            m_remote_ip->get_value(),
            static_cast<int>(m_remote_port->get_value()),
            m_ip_version->get_value(),
            resolved_ip
        ))
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, socket_error_t::error_address_invalid, "TCP-Client");
            close_and_clear_socket(sock);
            set_state(state_error);
            return false;
        }

        sockaddr_storage local_addr{};
        int              local_addr_len = 0;

        if (!resolve_address(adam::string_hashed(resolved_ip.c_str()), static_cast<int>(m_interface_port->get_value()), m_ip_version->get_value(), local_addr, local_addr_len, SOCK_STREAM))
        {
            log_network_message(log::error, log_event::tcp_address_resolution_failed, "TCP-Client", std::format("({}:{})", resolved_ip.c_str(), m_interface_port->get_value()));
            close_and_clear_socket(sock);
            set_state(state_error);
            return false;
        }

        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == socket_error_val)
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, resolve_socket_error(get_last_error()), "TCP-Client");
            close_and_clear_socket(sock);
            set_state(state_error);
            return false;
        }

        resolve_active_ip(sock);

        // --- Non-blocking connect ---
        if (!set_nonblocking(sock, true))
        {
            log_network_message(log::error, log_event::socket_option_failed, "TCP-Client");
            close_and_clear_socket(sock);
            set_state(state_error);
            return false;
        }

        int conn_res = ::connect(sock, reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        if (conn_res != socket_error_val)
        {
            {
                spinlock::guard lock(m_spinlock);
                m_socket = static_cast<uintptr_t>(sock);
            }
            log_network_message
            (
                log::info, 
                log_event::tcp_client_connected, 
                "TCP-Client",
                std::format("{} ({}) -> {}:{}", get_active_interface().c_str(), get_active_ip().c_str(), m_remote_ip->get_value().c_str(), m_remote_port->get_value())
            );
            return true;
        }

        int err = get_last_error();
        if (!is_blocking_error(err))
        {
            log_network_socket_error(log::warning, log_event::socket_connect_failed, resolve_socket_error(err), "TCP-Client");
            close_and_clear_socket(sock);
            return false;
        }

        // EINPROGRESS / WSAEWOULDBLOCK - wait with select()
        fd_set write_fds, err_fds;
        int64_t timeout_ms = m_reconnect_interval_ms->get_value();
        int select_res = 0;
        int64_t elapsed_ms = 0;

        while (elapsed_ms < timeout_ms && is_running())
        {
            timeval tv{};
            tv.tv_sec  = 0;
            tv.tv_usec = 50000; // 50ms

            FD_ZERO(&write_fds); FD_ZERO(&err_fds);
            FD_SET(sock, &write_fds);
            FD_SET(sock, &err_fds);

            select_res = ::select(static_cast<int>(sock) + 1, nullptr, &write_fds, &err_fds, &tv);
            if (select_res != 0)
            {
                break;
            }
            elapsed_ms += 50;
        }

        if (!is_running())
        {
            close_and_clear_socket(sock);
            return false;
        }

        if (select_res < 0)
        {
            log_network_socket_error(log::warning, log_event::socket_connect_failed, resolve_socket_error(get_last_error()), "TCP-Client");
            close_and_clear_socket(sock);
            return false;
        }

        if (select_res == 0)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            int err_code = WSAETIMEDOUT;
            #else
            int err_code = ETIMEDOUT;
            #endif
            log_network_socket_error(log::warning, log_event::socket_connect_failed, resolve_socket_error(err_code), "TCP-Client");
            close_and_clear_socket(sock);
            return false;
        }

        if (FD_ISSET(sock, &err_fds))
        {
            int err_code = 0;
            #if defined(ADAM_PLATFORM_WINDOWS)
            int opt_len = sizeof(err_code);
            #else
            socklen_t opt_len = sizeof(err_code);
            #endif
            ::getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err_code), &opt_len);
            log_network_socket_error(log::warning, log_event::socket_connect_failed, resolve_socket_error(err_code), "TCP-Client");
            close_and_clear_socket(sock);
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
            log_network_socket_error(log::warning, log_event::socket_connect_failed, resolve_socket_error(sock_err), "TCP-Client");
            close_and_clear_socket(sock);
            return false;
        }

        {
            spinlock::guard lock(m_spinlock);
            m_socket = static_cast<uintptr_t>(sock);
        }

        log_network_message
        (
            log::info, 
            log_event::tcp_client_connected, 
            "TCP-Client",
            std::format("{} ({}) -> {}:{}", get_active_interface().c_str(), get_active_ip().c_str(), m_remote_ip->get_value().c_str(), m_remote_port->get_value())
        );

        return true;
    }

    bool port_tcp_client::read(buffer*& buff)
    {
        while (is_running())
        {
            socket_t sock = static_cast<socket_t>(m_socket);

            if (sock == invalid_socket_val)
            {
                set_state(state_starting);

                auto start_time = std::chrono::steady_clock::now();
                if (connect())
                {
                    set_state(state_running);
                    continue;
                }
                auto end_time   = std::chrono::steady_clock::now();

                int64_t interval = m_reconnect_interval_ms->get_value();
                auto    elapsed  = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                int64_t sleep_ms = interval - elapsed;

                if (sleep_ms > 0)
                {
                    int64_t slept = 0;
                    while (slept < sleep_ms && is_running())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        slept += 50;
                    }
                }
                return false;
            }

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            timeval tv{ 0, 100000 };
            int rv = ::select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &tv);

            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;

                reset_socket_with_log();
                return false;
            }

            if (rv == 0) continue;

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

                reset_socket_with_log(log::warning, true);
                return false;
            }

            if (chunk == 0)
            {
                reset_socket_with_log(log::info, true);
                return false;
            }

            buff = buffer_manager::get().request_buffer(chunk);
            if (!buff) return false;

            std::memcpy(buff->data_as<uint8_t>(), temp_buf, chunk);
            buff->set_size(chunk);
            buff->set_timestamp();
            return true;
        }

        return false;
    }

    bool port_tcp_client::write(buffer* buff)
    {
        if (!buff) return false;
        if (buff->get_size() == 0) return false;
        if (get_state() != state_running) return false;

        if (!send_all(static_cast<socket_t>(m_socket), buff->get_data_as<char>(), buff->get_size()))
        {
            log_network_socket_error(log::warning, log_event::tcp_client_send_failed, resolve_socket_error(get_last_error()), "TCP-Client");
            return false;
        }

        return true;
    }

} // namespace adam::modules::network
