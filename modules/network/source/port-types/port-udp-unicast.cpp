#include "data/port-types/port-udp-unicast.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-network.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    // =========================================================================
    // Static user-parameter definition
    // =========================================================================

    const configuration_parameter_list& port_udp_unicast::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            // --- interface ---
            auto local_iface = create_interface_parameter();
            local_iface->set_description(language_english, "Local interface to bind to (auto = automatic)."_ct);
            local_iface->set_description(language_german,  "Lokale Schnittstelle zur Bindung (auto = automatisch)."_ct);
            up->add(std::move(local_iface));

            // --- interface_port ---
            auto local_port = std::make_unique<adam::configuration_parameter_integer>("interface_port"_ct, 0);
            local_port->set_description(language_english, "Local port to bind to (0 = any)."_ct);
            local_port->set_description(language_german,  "Lokaler Port zur Bindung (0 = beliebig)."_ct);
            up->add(std::move(local_port));

            // --- remote_ip ---
            auto remote_ip = std::make_unique<adam::configuration_parameter_string>("remote_ip"_ct, "127.0.0.1"_ct, create_ip_regex_parameter());
            remote_ip->set_description(language_english, "Destination IP address for outgoing datagrams."_ct);
            remote_ip->set_description(language_german,  "Ziel-IP-Adresse für ausgehende Datagramme."_ct);
            up->add(std::move(remote_ip));

            // --- remote_port ---
            auto remote_port = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port->set_description(language_english, "Destination port for outgoing datagrams."_ct);
            remote_port->set_description(language_german,  "Zielport für ausgehende Datagramme."_ct);
            up->add(std::move(remote_port));

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

    port_udp_unicast::port_udp_unicast(const string_hashed& item_name)
        : port_udp_base(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_udp_unicast::get_user_parameters());

        // Wire base-class shared parameter pointers.
        init_base_params(/*has_ip_version=*/true);

        // Wire unicast-specific parameter pointers.
        auto up    = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_remote_ip   = up->get<adam::configuration_parameter_string>("remote_ip"_ct);
        m_remote_port = up->get<adam::configuration_parameter_integer>("remote_port"_ct);
    }

    port_udp_unicast::~port_udp_unicast()
    {
        stop();
    }

    // =========================================================================
    // start() — create and bind the UDP socket
    // =========================================================================

    bool port_udp_unicast::start()
    {
        auto*    ctrl    = get_controller();
        language lang = ctrl->get_language();

        // --- Resolve the local bind address ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;

        std::string resolved_ip;
        if (!resolve_local_interface_to_ip(m_interface->get_value().c_str(),
                                           m_remote_ip->get_value().c_str(),
                                           static_cast<int>(m_remote_port->get_value()),
                                           m_ip_version->get_value(),
                                           resolved_ip) ||
            !resolve_address(adam::string_hashed(resolved_ip.c_str()),
                             static_cast<int>(m_interface_port->get_value()),
                             m_ip_version->get_value(), local_addr, local_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Unicast: {} ({})", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang), m_interface->get_value().c_str()));
            return false;
        }

        // --- Create the UDP socket ---
        socket_t sock = ::socket(local_addr.ss_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Unicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        // --- Opt into dual-stack when ip_version is "auto" and family is IPv6 ---
        apply_dual_stack_if_auto(sock, local_addr.ss_family, m_ip_version->get_value());

        // --- Bind to the local address ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == SOCKET_ERROR_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("[{}] UDP-Unicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        // --- Switch to non-blocking so the read loop can use select() timeouts ---
        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("[{}] UDP-Unicast: {}", get_name().c_str(), get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_socket = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("[{}] UDP-Unicast: {} (Port {})", get_name().c_str(), get_event_log_text(log_event::socket_bind_success, lang), m_interface_port->get_value()));

        return port::start();
    }

    // =========================================================================
    // write() — send a datagram to the configured remote address
    // =========================================================================

    bool port_udp_unicast::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        auto*    ctrl   = get_controller();
        language lang   = ctrl ? ctrl->get_language() : language_english;

        // --- Resolve the destination address at send-time (allows dynamic changes) ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_remote_ip->get_value(),
                             static_cast<int>(m_remote_port->get_value()),
                             m_ip_version->get_value(), dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            // Destination address is invalid — log and drop the packet.
            ctrl->log(log::warning, std::format("[{}] UDP-Unicast: {} ({}:{})", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), m_remote_ip->get_value().c_str(), m_remote_port->get_value()));
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
            ctrl->log(log::warning, std::format("[{}] UDP-Unicast: {} — {}", get_name().c_str(), get_event_log_text(log_event::udp_send_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }

} // namespace adam::modules::network
