#include "data/port-types/port-udp-broadcast.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module-network.hpp"

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

            // --- broadcast_ip ---
            auto broadcast_ip = std::make_unique<adam::configuration_parameter_string>("broadcast_ip"_ct, "auto"_ct, create_ip_regex_parameter());
            broadcast_ip->set_description(language_english, "Broadcast destination address (e.g. 255.255.255.255, auto, or subnet broadcast)."_ct);
            broadcast_ip->set_description(language_german,  "Broadcast-Zieladresse (z.B. 255.255.255.255, auto oder Subnet-Broadcast)."_ct);
            up->add(std::move(broadcast_ip));

            // --- remote_port ---
            auto remote_port = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port->set_description(language_english, "Destination port for outgoing broadcast datagrams."_ct);
            remote_port->set_description(language_german,  "Zielport für ausgehende Broadcast-Datagramme."_ct);
            up->add(std::move(remote_port));

            // Note: No ip_version parameter - UDP broadcast is IPv4-only.

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

        add_parameters(port_udp_broadcast::get_user_parameters());

        // Wire base-class shared pointers (no ip_version - broadcast is IPv4 only).
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
    // start() - create socket, enable SO_BROADCAST, bind
    // =========================================================================

    bool port_udp_broadcast::start()
    {
        set_state(state_starting);
        // Broadcast is always IPv4 - use the ipv4 hash constant.
        static const adam::string_hashed ipv4_ver("ipv4"_ct);

        // --- Resolve local bind address (always IPv4) ---
        sockaddr_storage local_addr{};
        int              local_addr_len = 0;
        std::string      resolved_ip;

        if (!resolve_bind_address(m_interface->get_value(),
                                  m_broadcast_ip->get_value(),
                                  static_cast<int>(m_remote_port->get_value()),
                                  static_cast<int>(m_interface_port->get_value()),
                                  ipv4_ver, SOCK_DGRAM,
                                  local_addr, local_addr_len, resolved_ip))
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, socket_error_t::error_address_invalid, "UDP-Broadcast");
            return false;
        }

        // --- Create IPv4 UDP socket ---
        socket_t sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == invalid_socket_val)
        {
            log_network_socket_error(log::error, log_event::socket_creation_failed, resolve_socket_error(get_last_error()), "UDP-Broadcast");
            return false;
        }

        // --- Enable SO_BROADCAST ---
        #if defined(ADAM_PLATFORM_WINDOWS)
        BOOL broadcast_val = TRUE;
        #else
        int  broadcast_val = 1;
        #endif
        if (::setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                         reinterpret_cast<const char*>(&broadcast_val),
                         sizeof(broadcast_val)) == socket_error_val)
        {
            log_network_socket_error(log::error, log_event::socket_option_failed, resolve_socket_error(get_last_error()), "UDP-Broadcast");
            close_and_clear_socket(sock);
            return false;
        }

        // --- Allow address reuse ---
        int reuse = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        // --- Bind ---
        if (::bind(sock, reinterpret_cast<sockaddr*>(&local_addr), local_addr_len) == socket_error_val)
        {
            log_network_socket_error(log::error, log_event::socket_bind_failed, resolve_socket_error(get_last_error()), "UDP-Broadcast");
            close_and_clear_socket(sock);
            return false;
        }

        resolve_active_ip(sock);

        // --- Resolve broadcast IP ---
        std::string target_broadcast = m_broadcast_ip->get_value();
        if (target_broadcast.empty() || target_broadcast == "auto")
        {
            target_broadcast = "255.255.255.255";
            std::string active_ip = get_active_ip();
            if (!active_ip.empty() && active_ip != "0.0.0.0")
            {
                auto& cache = get_adapter_cache();
                if (cache.empty()) refresh_adapter_cache();
                bool found = false;
                for (const auto& info : cache)
                {
                    for (size_t j = 0; j < info.ipv4_addresses.size(); ++j)
                    {
                        if (info.ipv4_addresses[j] == active_ip)
                        {
                            if (j < info.ipv4_broadcasts.size())
                            {
                                target_broadcast = info.ipv4_broadcasts[j];
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) break;
                }
            }
        }
        m_resolved_broadcast_ip = adam::string_hashed(target_broadcast.c_str());

        // --- Non-blocking ---
        if (!set_nonblocking(sock, true))
        {
            log_network_message(log::error, log_event::socket_option_failed, "UDP-Broadcast");
            close_and_clear_socket(sock);
            return false;
        }

        m_socket = static_cast<uintptr_t>(sock);

        log_network_message
        (
            log::info, 
            log_event::socket_bind_success, 
            "UDP-Broadcast",
            std::format
            (
                "{} ({}) Port {}, Broadcast: {}",
                get_active_interface().c_str(),
                get_active_ip().c_str(),
                m_active_port,
                m_resolved_broadcast_ip.c_str()
            )
        );

        return port::start();
    }

    // =========================================================================
    // write() - send a broadcast datagram
    // =========================================================================

    bool port_udp_broadcast::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == invalid_socket_val) return false;

        // Broadcast is always IPv4 - use the ipv4 hash constant.
        static const adam::string_hashed ipv4_ver("ipv4"_ct);

        // --- Resolve the broadcast destination address ---
        sockaddr_storage dest_addr{};
        int              dest_addr_len = 0;

        if (!resolve_address(m_resolved_broadcast_ip,
                             static_cast<int>(m_remote_port->get_value()),
                             ipv4_ver, dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            log_network_message(log::warning, log_event::udp_send_failed, "UDP-Broadcast",
                                std::format("({}:{})", m_resolved_broadcast_ip.c_str(), m_remote_port->get_value()));
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
            log_network_socket_error(log::warning, log_event::udp_send_failed, resolve_socket_error(get_last_error()), "UDP-Broadcast");
            return false;
        }

        return sent == static_cast<int>(size);
    }

} // namespace adam::modules::network
