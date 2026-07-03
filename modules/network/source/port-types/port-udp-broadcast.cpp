#include "data/port-types/port-udp-broadcast.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

    const configuration_parameter_list& port_udp_broadcast::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- local_interface (IPv4 only) ---
            auto local_iface = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, ""_ct, create_ip_regex_parameter());
            local_iface->set_description(language_english, "Local IPv4 interface to bind to (empty = all interfaces)."_ct);
            local_iface->set_description(language_german,  "Lokale IPv4-Schnittstelle zur Bindung (leer = alle Schnittstellen)."_ct);
            up->add(std::move(local_iface));

            // --- local_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("local_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind to (0 = any)."_ct);
            local_port->set_description(language_german,  "Lokaler Port zur Bindung (0 = beliebig)."_ct);
            up->add(std::move(local_port));

            // --- broadcast_ip ---
            auto broadcast_ip = std::make_unique<adam::configuration_parameter_string>("broadcast_ip"_ct, "255.255.255.255"_ct, create_ip_regex_parameter());
            broadcast_ip->set_description(language_english, "Broadcast destination address (e.g. 255.255.255.255 or subnet broadcast)."_ct);
            broadcast_ip->set_description(language_german,  "Broadcast-Zieladresse (z.B. 255.255.255.255 oder Subnet-Broadcast)."_ct);
            up->add(std::move(broadcast_ip));

            // --- remote_port ---
            auto remote_port = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port->set_description(language_english, "Destination port for outgoing broadcast datagrams."_ct);
            remote_port->set_description(language_german,  "Zielport für ausgehende Broadcast-Datagramme."_ct);
            up->add(std::move(remote_port));

            // Note: No ip_version parameter — UDP broadcast is IPv4-only.

            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    // =========================================================================
    // Constructor / Destructor
    // =========================================================================

    port_udp_broadcast::port_udp_broadcast(const string_hashed& item_name)
        : port_udp_base(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_udp_broadcast::get_user_parameters());

        // Wire base-class shared pointers (no ip_version — broadcast is IPv4 only).
        init_base_params(/*has_ip_version=*/false);

        // Wire broadcast-specific parameter pointers.
        auto up       = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_broadcast_ip = up->get<adam::configuration_parameter_string>("broadcast_ip"_ct);
        m_remote_port  = up->get<adam::configuration_parameter_integer>("remote_port"_ct);
    }

    port_udp_broadcast::~port_udp_broadcast()
    {
        stop();
    }

    // =========================================================================
    // start() — create socket, enable SO_BROADCAST, bind
    // =========================================================================

    bool port_udp_broadcast::start()
    {
        auto*    ctrl = get_controller();
        language lang = ctrl->get_language();

        // Broadcast is always IPv4 — use the ipv4 hash constant.
        static const adam::string_hashed ipv4_ver("ipv4"_ct);

        // --- Resolve local bind address (always IPv4) ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;

        if (!resolve_address(m_local_interface->get_value(),
                             static_cast<int>(m_local_port->get_value()),
                             ipv4_ver, local_addr, local_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Broadcast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang)));
            return false;
        }

        // --- Create IPv4 UDP socket ---
        socket_t sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Broadcast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // --- Enable SO_BROADCAST so we can send to broadcast addresses ---
        #if defined(ADAM_PLATFORM_WINDOWS)
        BOOL broadcast_val = TRUE;
        #else
        int  broadcast_val = 1;
        #endif
        if (::setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                         reinterpret_cast<const char*>(&broadcast_val),
                         sizeof(broadcast_val)) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Broadcast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_option_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Allow address reuse so multiple listeners can share the same port ---
        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        // --- Bind to the local address ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Broadcast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Switch to non-blocking so the read loop can use select() timeouts ---
        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Broadcast: {}", get_name().c_str(), get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_socket = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("[{}] UDP-Broadcast: {} (Port {})", get_name().c_str(), get_event_log_text(log_event::socket_bind_success, lang), m_local_port->get_value()));

        return port::start();
    }

    // =========================================================================
    // write() — send a broadcast datagram
    // =========================================================================

    bool port_udp_broadcast::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        auto*    ctrl = get_controller();
        language lang = ctrl->get_language();

        // Broadcast is always IPv4 — use the ipv4 hash constant.
        static const adam::string_hashed ipv4_ver("ipv4"_ct);

        // --- Resolve the broadcast destination address (always IPv4) ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_broadcast_ip->get_value(),
                             static_cast<int>(m_remote_port->get_value()),
                             ipv4_ver, dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::warning, std::format("[{}] UDP-Broadcast: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), m_broadcast_ip->get_value().c_str(), m_remote_port->get_value()));
            return false;
        }

        const char* data = buff->data_as<const char>();
        size_t      size = buff->get_size();

        #if defined(ADAM_PLATFORM_WINDOWS)
        int sent = ::sendto(sock, data, static_cast<int>(size), 0,
                            reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        #else
        int sent = ::sendto(sock, data, size, 0,
                            reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        #endif

        if (sent < 0)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::warning, std::format("[{}] UDP-Broadcast: {} — {}", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }

} // namespace adam::modules::network
