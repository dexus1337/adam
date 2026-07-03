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
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

    const configuration_parameter_list& port_tcp_server::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- local_interface ---
            auto local_iface = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, ""_ct, create_ip_regex_parameter());
            local_iface->set_description(language_english, "Local interface IP to listen on (empty = all interfaces)."_ct);
            local_iface->set_description(language_german,  "Lokale Schnittstellen-IP zum Lauschen (leer = alle Schnittstellen)."_ct);
            up->add(std::move(local_iface));

            // --- local_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("local_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind and listen on."_ct);
            local_port->set_description(language_german,  "Lokaler Port zum Binden und Lauschen."_ct);
            up->add(std::move(local_port));

            // --- tcp_nodelay ---
            auto nodelay = std::make_unique<adam::configuration_parameter_boolean>("tcp_nodelay"_ct, true);
            nodelay->set_description(language_english, "Apply TCP_NODELAY to each accepted client socket (lower latency)."_ct);
            nodelay->set_description(language_german,  "TCP_NODELAY auf akzeptierten Client-Sockets anwenden (geringere Latenz)."_ct);
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

    port_tcp_server::port_tcp_server(const string_hashed& item_name)
        : port_in_out(item_name)
        , m_listener(static_cast<uintptr_t>(INVALID_SOCKET_VAL))
        , m_clients()
        , m_clients_mutex()
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_tcp_server::get_user_parameters());

        // Cache user-parameter pointers (read-only after construction).
        auto up          = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_local_interface = up->get<adam::configuration_parameter_string>("local_interface"_ct);
        m_local_port      = up->get<adam::configuration_parameter_integer>("local_port"_ct);
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
        auto*      ctrl   = get_controller();
        language lang = ctrl->get_language();

        // --- Resolve the local listen address ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;

        if (!resolve_address(m_local_interface->get_value(),
                             static_cast<int>(m_local_port->get_value()),
                             m_ip_version->get_value(), local_addr, local_addr_len, SOCK_STREAM))
        {
            ctrl->log(log::error, std::format("[{}] TCP-Server: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang)));
            return false;
        }

        // --- Create the TCP listener socket ---
        socket_t sock = ::socket(local_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] TCP-Server: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // --- Reuse address so we can restart quickly after a crash ---
        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        // --- Dual-stack support when ip_version is "auto" ---
        apply_dual_stack_if_auto(sock, local_addr.ss_family, m_ip_version->get_value());

        // --- Bind ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] TCP-Server: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Listen ---
        if (::listen(sock, SOMAXCONN) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] TCP-Server: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_listen_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Non-blocking so the read loop can time out and check the running state ---
        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("[{}] TCP-Server: {}", get_name().c_str(), get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_listener = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("[{}] TCP-Server: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::tcp_server_started, lang), m_local_interface->get_value().c_str(), m_local_port->get_value()));

        return port::start();
    }

    // =========================================================================
    // stop() — close listener and all client sockets
    // =========================================================================

    bool port_tcp_server::stop()
    {
        // --- Close the listener socket (prevents new connections) ---
        socket_t listener_sock = static_cast<socket_t>(m_listener);
        if (listener_sock != INVALID_SOCKET_VAL)
        {
            close_socket(listener_sock);
            m_listener = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
        }

        // --- Atomically drain the client list under the spinlock ---
        std::vector<uintptr_t> clients_to_close;
        {
            adam::spinlock::guard lock(m_clients_mutex);
            clients_to_close = m_clients;
            m_clients.clear();
        }

        // --- Close client sockets outside the spinlock (syscalls must not be locked) ---
        for (auto c : clients_to_close)
        {
            socket_t cs = static_cast<socket_t>(c);
            close_socket(cs);
        }

        auto* ctrl = get_controller();
        language lang = ctrl->get_language();
        ctrl->log(log::info, std::format("[{}] TCP-Server: {}", get_name().c_str(), get_event_log_text(log_event::tcp_server_stopped, lang)));

        return port::stop();
    }

    // =========================================================================
    // read() — accept new clients and receive data from existing ones
    // =========================================================================

    bool port_tcp_server::read(buffer*& buff)
    {
        auto*    ctrl = get_controller();
        language lang = ctrl->get_language();

        while (is_running())
        {
            socket_t listener_sock = static_cast<socket_t>(m_listener);
            if (listener_sock == INVALID_SOCKET_VAL) return false;

            // --- Build the fd_set from the listener and all current clients ---
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listener_sock, &read_fds);
            int max_fd = static_cast<int>(listener_sock);

            // Snapshot the client list so we don't hold the spinlock during select().
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

            // 100 ms timeout keeps the thread responsive to stop().
            timeval tv{ 0, 100000 };
            int rv = ::select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);

            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;
                return false;
            }

            if (rv > 0)
            {
                // --- 1. Check for new incoming connections on the listener ---
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

                    if (client_sock != INVALID_SOCKET_VAL)
                    {
                        // Configure the accepted client socket.
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

                        // Register the new client under the spinlock.
                        {
                            adam::spinlock::guard lock(m_clients_mutex);
                            m_clients.push_back(static_cast<uintptr_t>(client_sock));
                        }
                        ctrl->log(log::info, std::format("[{}] TCP-Server: {}", get_name().c_str(), get_event_log_text(log_event::tcp_server_client_connected, lang)));
                    }
                    else
                    {
                        int err = get_last_error();
                        if (!is_blocking_error(err))
                        {
                            socket_error_t err_resolved = resolve_socket_error(err);
                            ctrl->log(log::warning, std::format("[{}] TCP-Server: {} — {}", get_name().c_str(), get_event_log_text(log_event::tcp_server_accept_failed, lang), get_error_log_text(err_resolved, lang)));
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
                        // Data received — wrap it in a buffer and return.
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
                        // chunk == 0 (graceful close) or chunk < 0 (error) — remove client.
                        {
                            adam::spinlock::guard lock(m_clients_mutex);
                            auto it = std::find(m_clients.begin(), m_clients.end(), c);
                            if (it != m_clients.end())
                                m_clients.erase(it);
                        }
                        close_socket(cs);
                        ctrl->log(log::info, std::format("[{}] TCP-Server: {}", get_name().c_str(), get_event_log_text(log_event::tcp_server_client_disconnected, lang)));
                    }
                }
            }
            // rv == 0: timeout — loop back to check the running state.
        }

        return false;
    }

    // =========================================================================
    // write() — broadcast to all connected clients
    // =========================================================================

    bool port_tcp_server::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        // Snapshot the client list — hold the spinlock only for the copy.
        std::vector<uintptr_t> clients_copy;
        {
            adam::spinlock::guard lock(m_clients_mutex);
            clients_copy = m_clients;
        }

        if (clients_copy.empty()) return true; // No clients — broadcast is a no-op.

        const char* data = buff->data_as<const char>();
        size_t      size = buff->get_size();

        // --- Send to every client; collect failures ---
        std::vector<uintptr_t> failed_clients;
        for (auto c : clients_copy)
        {
            socket_t cs = static_cast<socket_t>(c);
            #if defined(ADAM_PLATFORM_WINDOWS)
            int sent = ::send(cs, data, static_cast<int>(size), 0);
            #else
            int sent = ::send(cs, data, size, 0);
            #endif

            if (sent < 0 && !is_blocking_error(get_last_error()))
                failed_clients.push_back(c);
        }

        // --- Remove and close failed clients ---
        if (!failed_clients.empty())
        {
            // Remove from the authoritative list under the spinlock.
            {
                adam::spinlock::guard lock(m_clients_mutex);
                for (auto fc : failed_clients)
                {
                    auto it = std::find(m_clients.begin(), m_clients.end(), fc);
                    if (it != m_clients.end())
                        m_clients.erase(it);
                }
            }

            // Close sockets and log outside the spinlock.
            auto*    ctrl = get_controller();
            language lang = ctrl->get_language();
            for (auto fc : failed_clients)
            {
                socket_t fcs = static_cast<socket_t>(fc);
                close_socket(fcs);
                ctrl->log(log::info, std::format("[{}] TCP-Server: {}", get_name().c_str(), get_event_log_text(log_event::tcp_server_client_disconnected, lang)));
            }
        }

        return true;
    }

} // namespace adam::modules::network
