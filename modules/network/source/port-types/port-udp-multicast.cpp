#include "data/port-types/port-udp-multicast.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

    const configuration_parameter_list& port_udp_multicast::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- local_interface ---
            auto local_iface = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, ""_ct, create_ip_regex_parameter());
            local_iface->set_description(language_english, "Local interface IP to bind to (empty = any/all interfaces)."_ct);
            local_iface->set_description(language_german,  "Lokale Schnittstellen-IP zur Bindung (leer = beliebig/alle Schnittstellen)."_ct);
            up->add(std::move(local_iface));

            // --- local_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("local_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind to (usually the same as multicast_port)."_ct);
            local_port->set_description(language_german,  "Lokaler Port zur Bindung (normalerweise identisch mit multicast_port)."_ct);
            up->add(std::move(local_port));

            // --- multicast_ip ---
            auto multicast_ip = std::make_unique<adam::configuration_parameter_string>("multicast_ip"_ct, "239.0.0.1"_ct, create_ip_regex_parameter());
            multicast_ip->set_description(language_english, "Multicast group address (e.g. 239.0.0.1 for IPv4 or ff02::1 for IPv6)."_ct);
            multicast_ip->set_description(language_german,  "Multicast-Gruppenadresse (z.B. 239.0.0.1 für IPv4 oder ff02::1 für IPv6)."_ct);
            up->add(std::move(multicast_ip));

            // --- multicast_port ---
            auto multicast_port = std::make_unique<adam::configuration_parameter_integer>("multicast_port"_ct, 0);
            multicast_port->set_description(language_english, "Destination multicast port for outgoing datagrams."_ct);
            multicast_port->set_description(language_german,  "Ziel-Multicast-Port für ausgehende Datagramme."_ct);
            up->add(std::move(multicast_port));

            // --- ttl ---
            auto ttl = std::make_unique<adam::configuration_parameter_integer>("ttl"_ct, 1);
            ttl->set_description(language_english, "Multicast Time-To-Live (IPv4) or hop limit (IPv6). Default 1 = link-local."_ct);
            ttl->set_description(language_german,  "Multicast Time-To-Live (IPv4) oder Hop-Limit (IPv6). Standard 1 = link-lokal."_ct);
            up->add(std::move(ttl));

            // --- loopback ---
            auto loopback = std::make_unique<adam::configuration_parameter_boolean>("loopback"_ct, true);
            loopback->set_description(language_english, "Enable multicast loopback so sent datagrams are also received locally."_ct);
            loopback->set_description(language_german,  "Multicast-Loopback aktivieren, damit gesendete Datagramme auch lokal empfangen werden."_ct);
            up->add(std::move(loopback));

            // --- ip_version (auto / ipv4 / ipv6) ---
            add_ip_version_param(up.get());

            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    // =========================================================================
    // Constructor / Destructor
    // =========================================================================

    port_udp_multicast::port_udp_multicast(const string_hashed& item_name)
        : port_udp_base(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_udp_multicast::get_user_parameters());

        // Wire base-class shared parameter pointers.
        init_base_params(/*has_ip_version=*/true);

        // Wire multicast-specific parameter pointers.
        auto up         = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_multicast_ip   = up->get<adam::configuration_parameter_string>("multicast_ip"_ct);
        m_multicast_port = up->get<adam::configuration_parameter_integer>("multicast_port"_ct);
        m_ttl            = up->get<adam::configuration_parameter_integer>("ttl"_ct);
        m_loopback       = up->get<adam::configuration_parameter_boolean>("loopback"_ct);
    }

    port_udp_multicast::~port_udp_multicast()
    {
        stop();
    }

    // =========================================================================
    // start() — create socket, join multicast group, bind
    // =========================================================================

    bool port_udp_multicast::start()
    {
        auto*      ctrl   = get_controller();
        language lang = ctrl->get_language();

        // --- Resolve local bind address ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;

        if (!resolve_address(m_local_interface->get_value(),
                             static_cast<int>(m_local_port->get_value()),
                             m_ip_version->get_value(), local_addr, local_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang)));
            return false;
        }

        // --- Create the UDP socket ---
        socket_t sock = ::socket(local_addr.ss_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // --- Allow address reuse so multiple receivers can share the same multicast port ---
        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        // --- Bind to the local address ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Join the multicast group and configure TTL/hops and loopback ---
        if (local_addr.ss_family == AF_INET)
        {
            // IPv4 multicast
            ip_mreq mreq{};
            if (::inet_pton(AF_INET, m_multicast_ip->get_value().c_str(), &mreq.imr_multiaddr) <= 0)
            {
                ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} ({})", get_name().c_str(), get_event_log_text(log_event::multicast_join_failed, lang), get_event_log_text(log_event::multicast_address_invalid, lang)));
                close_socket(sock);
                return false;
            }
            ::inet_pton(AF_INET, m_local_interface->get_value().c_str(), &mreq.imr_interface);

            if (::setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                             reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == SOCKET_ERROR_VAL)
            {
                socket_error_t err = resolve_socket_error(get_last_error());
                ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::multicast_join_failed, lang), get_error_log_text(err, lang)));
                close_socket(sock);
                return false;
            }

            // TTL — limits the multicast radius (1 = link-local only).
            int ttl_val = static_cast<int>(m_ttl->get_value());
            ::setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                         reinterpret_cast<const char*>(&ttl_val), sizeof(ttl_val));

            // Loopback — controls whether the sending host receives its own multicast.
            #if defined(ADAM_PLATFORM_WINDOWS)
            BOOL loop = m_loopback->get_value() ? TRUE : FALSE;
            #else
            u_char loop = m_loopback->get_value() ? 1 : 0;
            #endif
            ::setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                         reinterpret_cast<const char*>(&loop), sizeof(loop));
        }
        else // AF_INET6
        {
            // IPv6 multicast
            ipv6_mreq mreq6{};
            if (::inet_pton(AF_INET6, m_multicast_ip->get_value().c_str(), &mreq6.ipv6mr_multiaddr) <= 0)
            {
                ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} ({})", get_name().c_str(), get_event_log_text(log_event::multicast_join_failed, lang), get_event_log_text(log_event::multicast_address_invalid, lang)));
                close_socket(sock);
                return false;
            }
            mreq6.ipv6mr_interface = 0; // Use the default interface index.

            if (::setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                             reinterpret_cast<const char*>(&mreq6), sizeof(mreq6)) == SOCKET_ERROR_VAL)
            {
                socket_error_t err = resolve_socket_error(get_last_error());
                ctrl->log(log::error, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::multicast_join_failed, lang), get_error_log_text(err, lang)));
                close_socket(sock);
                return false;
            }

            // Hop limit — IPv6 equivalent of IPv4 TTL.
            int ttl_val = static_cast<int>(m_ttl->get_value());
            ::setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                         reinterpret_cast<const char*>(&ttl_val), sizeof(ttl_val));

            // Loopback — IPv6 equivalent of IP_MULTICAST_LOOP.
            #if defined(ADAM_PLATFORM_WINDOWS)
            BOOL loop = m_loopback->get_value() ? TRUE : FALSE;
            #else
            u_int loop = m_loopback->get_value() ? 1 : 0;
            #endif
            ::setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                         reinterpret_cast<const char*>(&loop), sizeof(loop));
        }

        // --- Switch to non-blocking so the read loop can use select() timeouts ---
        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Multicast: {}", get_name().c_str(), get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_socket = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("[{}] UDP-Multicast: {} (Group {}, Port {})", get_name().c_str(), get_event_log_text(log_event::multicast_join_success, lang), m_multicast_ip->get_value().c_str(), m_local_port->get_value()));

        return port::start();
    }

    // =========================================================================
    // write() — send a datagram to the multicast group
    // =========================================================================

    bool port_udp_multicast::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        auto*      ctrl   = get_controller();
        language lang = ctrl->get_language();

        // --- Resolve the multicast group address at send-time ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_multicast_ip->get_value(),
                             static_cast<int>(m_multicast_port->get_value()),
                             m_ip_version->get_value(), dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::warning, std::format("[{}] UDP-Multicast: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), m_multicast_ip->get_value().c_str(), m_multicast_port->get_value()));
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
            ctrl->log(log::warning, std::format("[{}] UDP-Multicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }

} // namespace adam::modules::network
