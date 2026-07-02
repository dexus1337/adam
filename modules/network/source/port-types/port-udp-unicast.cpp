#include "data/port-types/port-udp-unicast.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "port-types/socket-helpers.hpp"

namespace adam::modules::network
{
    const configuration_parameter_list& port_udp_unicast::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto local_iface_param = std::make_unique<adam::configuration_parameter_string>("local_interface"_ct, "0.0.0.0"_ct);
            local_iface_param->set_description(language_english, "The local interface IP to bind to (e.g. 0.0.0.0 or ::)."_ct);
            local_iface_param->set_description(language_german, "Die lokale Schnittstellen-IP zur Bindung (z.B. 0.0.0.0 oder ::)."_ct);
            up->add(std::move(local_iface_param));

            auto local_port_param = std::make_unique<adam::configuration_parameter_integer>("local_port"_ct, 0);
            local_port_param->set_description(language_english, "The local port to bind to."_ct);
            local_port_param->set_description(language_german, "Der lokale Port zur Bindung."_ct);
            up->add(std::move(local_port_param));

            auto remote_ip_param = std::make_unique<adam::configuration_parameter_string>("remote_ip"_ct, "127.0.0.1"_ct);
            remote_ip_param->set_description(language_english, "The destination IP address for outgoing data."_ct);
            remote_ip_param->set_description(language_german, "Die Ziel-IP-Adresse für ausgehende Daten."_ct);
            up->add(std::move(remote_ip_param));

            auto remote_port_param = std::make_unique<adam::configuration_parameter_integer>("remote_port"_ct, 0);
            remote_port_param->set_description(language_english, "The destination port for outgoing data."_ct);
            remote_port_param->set_description(language_german, "Der Zielport für ausgehende Daten."_ct);
            up->add(std::move(remote_port_param));

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

    port_udp_unicast::port_udp_unicast(const string_hashed& item_name)
        : port_in_out(item_name), m_socket(static_cast<uintptr_t>(INVALID_SOCKET_VAL))
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        add_parameters(port_udp_unicast::get_user_parameters());

        auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_local_interface = user_params->get<adam::configuration_parameter_string>("local_interface"_ct);
        m_local_port = user_params->get<adam::configuration_parameter_integer>("local_port"_ct);
        m_remote_ip = user_params->get<adam::configuration_parameter_string>("remote_ip"_ct);
        m_remote_port = user_params->get<adam::configuration_parameter_integer>("remote_port"_ct);
        m_ip_version = user_params->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    port_udp_unicast::~port_udp_unicast()
    {
        stop();
    }

    bool port_udp_unicast::start()
    {
        auto* ctrl = get_controller();
        language lang = ctrl ? ctrl->get_language() : language_english;

        sockaddr_storage local_addr;
        int local_addr_len = 0;
        std::string ip_ver = m_ip_version->get_value().c_str();

        if (!resolve_address(m_local_interface->get_value().c_str(), static_cast<int>(m_local_port->get_value()), ip_ver, local_addr, local_addr_len, SOCK_DGRAM))
        {
            ctrl->log(log::error, std::format("UDP-Unicast: {} ({})", get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(socket_error_t::error_address_invalid, lang)));
            return false;
        }

        socket_t sock = ::socket(local_addr.ss_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET_VAL)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::error, std::format("UDP-Unicast: {} ({})", get_event_log_text(log_event::socket_creation_failed, lang), get_error_log_text(err, lang)));
            return false;
        }

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
            ctrl->log(log::error, std::format("UDP-Unicast: {} ({})", get_event_log_text(log_event::socket_bind_failed, lang), get_error_log_text(err, lang)));
            close_socket(sock);
            return false;
        }

        if (!set_nonblocking(sock, true))
        {
            ctrl->log(log::error, std::format("UDP-Unicast: {}", get_event_log_text(log_event::socket_option_failed, lang)));
            close_socket(sock);
            return false;
        }

        m_socket = static_cast<uintptr_t>(sock);

        ctrl->log(log::info, std::format("UDP-Unicast: {} (Port {})", get_event_log_text(log_event::socket_bind_success, lang), m_local_port->get_value()));

        return port::start();
    }

    bool port_udp_unicast::stop()
    {
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock != INVALID_SOCKET_VAL)
        {
            close_socket(sock);
            m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);
            
            auto* ctrl = get_controller();
            ctrl->log(log::info, "UDP-Unicast: socket closed.");
        }
        return port::stop();
    }

    bool port_udp_unicast::read(buffer*& buff)
    {
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        uint8_t temp_buf[65536];
        int bytes_read = 0;

        while (is_started())
        {
            sock = static_cast<socket_t>(m_socket);
            if (sock == INVALID_SOCKET_VAL) return false;

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
                return false;
            }
            else if (rv > 0)
            {
                sockaddr_storage from_addr;
                #if defined(ADAM_PLATFORM_WINDOWS)
                int from_len = sizeof(from_addr);
                int chunk = ::recvfrom(sock, reinterpret_cast<char*>(temp_buf), sizeof(temp_buf), 0, reinterpret_cast<sockaddr*>(&from_addr), &from_len);
                #else
                socklen_t from_len = sizeof(from_addr);
                int chunk = ::recvfrom(sock, temp_buf, sizeof(temp_buf), 0, reinterpret_cast<sockaddr*>(&from_addr), &from_len);
                #endif

                if (chunk < 0)
                {
                    int err = get_last_error();
                    if (is_blocking_error(err)) continue;
                    return false;
                }
                
                bytes_read = chunk;
                break;
            }
        }

        if (bytes_read > 0)
        {
            buff = buffer_manager::get().request_buffer(bytes_read);
            if (!buff) return false;

            std::memcpy(buff->data_as<uint8_t>(), temp_buf, bytes_read);
            buff->set_size(bytes_read);
            buff->set_timestamp();
            return true;
        }

        return false;
    }

    bool port_udp_unicast::write(buffer* buff)
    {
        if (!buff || buff->get_size() == 0) return false;

        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        auto* ctrl = get_controller();
        language lang = ctrl ? ctrl->get_language() : language_english;

        sockaddr_storage dest_addr;
        int dest_addr_len = 0;
        std::string ip_ver = m_ip_version->get_value().c_str();

        if (!resolve_address(m_remote_ip->get_value().c_str(), static_cast<int>(m_remote_port->get_value()), ip_ver, dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            return false;
        }

        const char* data = buff->data_as<const char>();
        size_t size = buff->get_size();

        #if defined(ADAM_PLATFORM_WINDOWS)
        int sent = ::sendto(sock, data, static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        #else
        int sent = ::sendto(sock, data, size, 0, reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len);
        #endif

        if (sent < 0)
        {
            socket_error_t err = resolve_socket_error(get_last_error());
            ctrl->log(log::warning, std::format("UDP-Unicast: Send failed ({})", get_error_log_text(err, lang)));
            return false;
        }

        return sent == static_cast<int>(size);
    }
}
