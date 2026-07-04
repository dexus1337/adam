#include "data/port-types/port-udp-base.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"

namespace adam::modules::network
{
    // =========================================================================
    // stop()
    // =========================================================================

    bool port_udp_base::stop()
    {
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock != INVALID_SOCKET_VAL)
        {
            close_socket(sock);
            m_socket = static_cast<uintptr_t>(INVALID_SOCKET_VAL);

            auto* ctrl = get_controller();
            language lang = ctrl->get_language();
            ctrl->log(log::info, std::format("[{}] {}", get_name().c_str(), get_event_log_text(log_event::udp_socket_closed, lang)));
        }
        return port::stop();
    }

    // =========================================================================
    // read()
    // =========================================================================

    bool port_udp_base::read(buffer*& buff)
    {
        // Snapshot the socket handle; if it becomes invalid mid-run (stop() was called),
        // the select() will fail and we will exit the loop cleanly.
        socket_t sock = static_cast<socket_t>(m_socket);
        if (sock == INVALID_SOCKET_VAL) return false;

        uint8_t temp_buf[65536];
        int     bytes_read = 0;

        while (is_running())
        {
            // Re-read the handle each iteration in case stop() was called.
            sock = static_cast<socket_t>(m_socket);
            if (sock == INVALID_SOCKET_VAL) return false;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            // 100 ms timeout keeps the thread responsive to stop().
            timeval tv{ 0, 100000 };

            int rv = ::select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &tv);
            if (rv < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err)) continue;
                return false; // Hard socket error — port will attempt to restart.
            }

            if (rv > 0)
            {
                // A datagram is waiting; receive it.
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
            // rv == 0: timeout — loop back and check the running state again.
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

    // =========================================================================
    // init_base_params()
    // =========================================================================

    void port_udp_base::init_base_params(bool has_ip_version)
    {
        auto up = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        m_interface      = up->get<adam::configuration_parameter_string>("interface"_ct);
        m_interface_port = up->get<adam::configuration_parameter_integer>("interface_port"_ct);
        if (has_ip_version)
            m_ip_version  = up->get<adam::configuration_parameter_string>("ip_version"_ct);
    }

    // =========================================================================
    // add_ip_version_param()
    // =========================================================================

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
