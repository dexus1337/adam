#include "data/port-types/port-udp-base.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"

namespace adam::modules::network
{

    bool port_udp_base::stop()
    {
        set_state(state_stopping);
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock != invalid_socket_val)
        {
            close_socket(sock);
            m_socket = static_cast<uintptr_t>(invalid_socket_val);
            log_network_message(log::info, log_event::udp_socket_closed, "");
        }

        m_active_ip.clear();

        return port::stop();
    }

    bool port_udp_base::read(buffer*& buff)
    {
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == invalid_socket_val) return false;

        uint8_t temp_buf[65536];
        int     bytes_read = 0;

        while (is_running())
        {
            sock = static_cast<socket_t>(m_socket);
            if (sock == invalid_socket_val) return false;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            timeval tv{ 0, 100000 };

            int rv = ::select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &tv);
            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;
                return false;
            }

            if (rv > 0)
            {
                sockaddr_storage from_addr{};
                #if defined(ADAM_PLATFORM_WINDOWS)
                int from_len = sizeof(from_addr);
                int chunk    = ::recvfrom(sock, reinterpret_cast<char*>(temp_buf),
                                          sizeof(temp_buf), 0,
                                          reinterpret_cast<sockaddr*>(&from_addr), &from_len);
                #else
                socklen_t from_len = sizeof(from_addr);
                int chunk          = ::recvfrom(sock, temp_buf,
                                                sizeof(temp_buf), 0,
                                                reinterpret_cast<sockaddr*>(&from_addr), &from_len);
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

    void port_udp_base::init_base_params(bool has_ip_version)
    {
        auto up = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_interface      = up->get<adam::configuration_parameter_string>("interface"_ct);
        m_interface_port = up->get<adam::configuration_parameter_integer>("interface_port"_ct);
        if (has_ip_version)
            m_ip_version  = up->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    void port_udp_base::add_ip_version_param(adam::configuration_parameter_list_sorted* up)
    {
        configuration_parameter_string::presets_container presets;
        presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("auto"_ct, "auto"_ct));
        presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("ipv4"_ct, "ipv4"_ct));
        presets.emplace("2"_ct, std::make_unique<adam::configuration_parameter_string>("ipv6"_ct, "ipv6"_ct));

        auto p = std::make_unique<adam::configuration_parameter_string>(
            "ip_version"_ct, "auto"_ct, std::move(presets));
        p->set_description(language_english, "IP version to use: auto, ipv4, or ipv6."_ct);
        p->set_description(language_german,  "Zu verwendende IP-Version: auto, ipv4 oder ipv6."_ct);
        up->add(std::move(p));
    }

} // namespace adam::modules::network
