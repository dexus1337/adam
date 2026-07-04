#include "data/port-types/port-tcp-client.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-network.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

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

    // =========================================================================
    // Constructor / Destructor
    // =========================================================================

    port_tcp_client::port_tcp_client(const string_hashed& item_name)
        : port_in_out(item_name)
        , m_socket(static_cast<uintptr_t>(INVALID_SOCKET_VAL))
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_tcp_client::get_user_parameters());

        // Cache user-parameter pointers (read-only after construction).
        auto up               = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_interface            = up->get<adam::configuration_parameter_string>("interface"_ct);
        m_remote_ip            = up->get<adam::configuration_parameter_string>("remote_ip"_ct);
        m_remote_port          = up->get<adam::configuration_parameter_integer>("remote_port"_ct);
        m_reconnect_interval_ms = up->get<adam::configuration_parameter_integer>("reconnect_interval_ms"_ct);
        m_tcp_nodelay          = up->get<adam::configuration_parameter_boolean>("tcp_nodelay"_ct);
        m_ip_version           = up->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    port_tcp_client::~port_tcp_client()
    {
        stop();
    }

    // =========================================================================
    // start()
    // =========================================================================

    bool port_tcp_client::start()
    {
        // The worker thread managed by port::start() will call read() in a loop,
        // which handles connection and reconnection internally.
        return port::start();
    }

    // =========================================================================
    // stop()
    // =========================================================================

    bool port_tcp_client::stop()
    {
        // Atomically take the socket handle under the spinlock so that write()
        // cannot observe a half-closed state.
        socket_t sock = static_cast<socket_t>(INVALID_SOCKET_VAL);
        {
            adam::spinlock::guard lock(m_write_mutex);
            sock     = static_cast<socket_t>(m_socket);
            m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
        }

        // Close the socket outside the spinlock — close_socket() is a syscall.
        if (sock != INVALID_SOCKET_VAL)
        {
            close_socket(sock);
            auto* ctrl = get_controller();
            language lang = ctrl->get_language();
            ctrl->log(log::info, std::format("[{}] TCP-Client: {}", get_name().c_str(), get_event_log_text(log_event::tcp_client_stopped, lang)));
        }

        return port::stop();
    }

    // =========================================================================
    // connect() — called by read() when the socket is invalid
    // =========================================================================

    bool port_tcp_client::connect()
    {
        auto*      ctrl   = get_controller();
        language   lang   = ctrl->get_language();


        // --- Resolve the remote server address ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_remote_ip->get_value(),
                             static_cast<int>(m_remote_port->get_value()),
                             m_ip_version->get_value(), dest_addr, dest_addr_len, SOCK_STREAM))
        {
            ctrl->log(log::error, std::format("[{}] TCP-Client: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::tcp_address_resolution_failed, lang), m_remote_ip->get_value().c_str(), m_remote_port->get_value()));
            return false;
        }

        // --- Create the TCP socket ---
        socket_t sock = ::socket(dest_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] TCP-Client: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // --- TCP_NODELAY — disable Nagle for latency-sensitive pipelines ---
        if (m_tcp_nodelay->get_value())
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            BOOL val = TRUE;
            #else
            int  val = 1;
            #endif
            ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                         reinterpret_cast<const char*>(&val), sizeof(val));
        }

        // --- Local interface bind ---
        std::string resolved_ip;
        if (resolve_local_interface_to_ip(m_interface->get_value().c_str(),
                                           m_remote_ip->get_value().c_str(),
                                           static_cast<int>(m_remote_port->get_value()),
                                           m_ip_version->get_value(),
                                           resolved_ip))
        {
            sockaddr_storage local_addr{};
            int              local_addr_len = 0;
            if (resolve_address(adam::string_hashed(resolved_ip.c_str()), 0,
                                m_ip_version->get_value(), local_addr, local_addr_len, SOCK_STREAM))
            {
                ::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len);
            }
        }

        // --- Non-blocking connect so we can respect a connect timeout / port stop ---
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
                // Hard connect failure (e.g. connection refused).
                socket_error_t err_resolved = resolve_socket_error(err);
                ctrl->log(log::error, std::format("[{}] TCP-Client: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }

            // Connect is in progress (EINPROGRESS / WSAEWOULDBLOCK) — wait with select().
            fd_set write_fds, err_fds;
            FD_ZERO(&write_fds); FD_ZERO(&err_fds);
            FD_SET(sock, &write_fds);
            FD_SET(sock, &err_fds);

            timeval tv{ 2, 0 }; // 2-second connect timeout.
            int select_res = ::select(static_cast<int>(sock) + 1, nullptr, &write_fds, &err_fds, &tv);

            if (select_res <= 0 || FD_ISSET(sock, &err_fds) || !is_running())
            {
                // Timeout, error, or the port was stopped while we waited.
                int err_code = 0;
                if (select_res == 0)
                {
                    #if defined(ADAM_PLATFORM_WINDOWS)
                    err_code = WSAETIMEDOUT;
                    #else
                    err_code = ETIMEDOUT;
                    #endif
                }
                else if (FD_ISSET(sock, &err_fds))
                {
                    #if defined(ADAM_PLATFORM_WINDOWS)
                    int opt_len = sizeof(err_code);
                    #else
                    socklen_t opt_len = sizeof(err_code);
                    #endif
                    ::getsockopt(sock, SOL_SOCKET, SO_ERROR,
                                 reinterpret_cast<char*>(&err_code), &opt_len);
                }
                socket_error_t err_resolved = resolve_socket_error(err_code);
                ctrl->log(log::error, std::format("[{}] TCP-Client: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }

            // Verify there is no pending socket-level error before declaring success.
            int sock_err = 0;
            #if defined(ADAM_PLATFORM_WINDOWS)
            int opt_len = sizeof(sock_err);
            #else
            socklen_t opt_len = sizeof(sock_err);
            #endif
            if (::getsockopt(sock, SOL_SOCKET, SO_ERROR,
                             reinterpret_cast<char*>(&sock_err), &opt_len) < 0 || sock_err != 0)
            {
                socket_error_t err_resolved = resolve_socket_error(sock_err);
                ctrl->log(log::error, std::format("[{}] TCP-Client: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_connect_failed, lang), get_error_log_text(err_resolved, lang)));
                close_socket(sock);
                return false;
            }
        }

        // --- Publish the connected socket under the spinlock ---
        {
            adam::spinlock::guard lock(m_write_mutex);
            m_socket = static_cast<uintptr_t>(sock);
        }

        ctrl->log(log::info, std::format("[{}] TCP-Client: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::tcp_client_connected, lang), m_remote_ip->get_value().c_str(), m_remote_port->get_value()));

        return true;
    }

    // =========================================================================
    // read() — connect + select/recv loop
    // =========================================================================

    bool port_tcp_client::read(buffer*& buff)
    {
        auto*    ctrl = get_controller();
        language lang = ctrl->get_language();

        while (is_running())
        {
            socket_t sock = static_cast<socket_t>(m_socket);

            if (sock == INVALID_SOCKET_VAL)
            {
                // No connection yet (or we disconnected) — attempt to connect.
                if (connect())
                {
                    set_state(state_running);
                    continue; // Connection succeeded; loop back to start reading.
                }

                // Connection failed — mark as connecting
                set_state(state_connecting);

                // Connection failed — wait the reconnect interval before retrying,
                // but wake up every 100 ms to check whether the port was stopped.
                int64_t interval = m_reconnect_interval_ms->get_value();
                if (interval <= 0) interval = 2000;

                int64_t slept = 0;
                while (slept < interval && is_running())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    slept += 100;
                }
                return false; // Signal port to call read() again.
            }

            // --- Wait for incoming data (100 ms timeout) ---
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            timeval tv{ 0, 100000 };
            int rv = ::select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &tv);

            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;

                // Hard socket error — invalidate and let next iteration reconnect.
                socket_t s_to_close = INVALID_SOCKET_VAL;
                {
                    adam::spinlock::guard lock(m_write_mutex);
                    s_to_close = static_cast<socket_t>(m_socket);
                    m_socket   = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                }
                if (s_to_close != INVALID_SOCKET_VAL) close_socket(s_to_close);
                return false;
            }

            if (rv > 0)
            {
                // Data is ready — receive it into a temporary stack buffer.
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

                    // Receive error — close and allow reconnect.
                    socket_t s_to_close = INVALID_SOCKET_VAL;
                    {
                        adam::spinlock::guard lock(m_write_mutex);
                        s_to_close = static_cast<socket_t>(m_socket);
                        m_socket   = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                    }
                    if (s_to_close != INVALID_SOCKET_VAL) close_socket(s_to_close);
                    ctrl->log(log::warning, std::format("[{}] TCP-Client: {}", get_name().c_str(), get_event_log_text(log_event::tcp_client_disconnected, lang)));
                    return false;
                }

                if (chunk == 0)
                {
                    // Server closed the connection gracefully (EOF).
                    socket_t s_to_close = INVALID_SOCKET_VAL;
                    {
                        adam::spinlock::guard lock(m_write_mutex);
                        s_to_close = static_cast<socket_t>(m_socket);
                        m_socket   = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
                    }
                    if (s_to_close != INVALID_SOCKET_VAL) close_socket(s_to_close);
                    ctrl->log(log::info, std::format("[{}] TCP-Client: {}", get_name().c_str(), get_event_log_text(log_event::tcp_client_disconnected, lang)));
                    return false;
                }

                // --- Wrap received bytes in a buffer and return it to the port framework ---
                buff = buffer_manager::get().request_buffer(chunk);
                if (!buff) return false;

                std::memcpy(buff->data_as<uint8_t>(), temp_buf, chunk);
                buff->set_size(chunk);
                buff->set_timestamp();
                return true;
            }
            // rv == 0: timeout — loop back to check the running state.
        }

        return false;
    }

    // =========================================================================
    // write() — send data to the connected server
    // =========================================================================

    bool port_tcp_client::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        // Read the socket handle atomically — hold the spinlock only for this one load.
        socket_t sock = static_cast<socket_t>(INVALID_SOCKET_VAL);
        {
            adam::spinlock::guard lock(m_write_mutex);
            sock = static_cast<socket_t>(m_socket);
        }
        if (sock == INVALID_SOCKET_VAL) return false; // Not connected.

        const char* data = buff->data_as<const char>();
        size_t      size = buff->get_size();

        #if defined(ADAM_PLATFORM_WINDOWS)
        int sent = ::send(sock, data, static_cast<int>(size), 0);
        #else
        int sent = ::send(sock, data, size, 0);
        #endif

        if (sent < 0)
        {
            auto*    ctrl = get_controller();
            language lang = ctrl->get_language();
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::warning, std::format("[{}] TCP-Client: {} — {}", get_name().c_str(), get_event_log_text(log_event::tcp_client_send_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }

} // namespace adam::modules::network
