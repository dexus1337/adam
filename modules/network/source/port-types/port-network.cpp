#include "data/port-types/port-network.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "module/module-network.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <format>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <array>

#if defined(ADAM_PLATFORM_WINDOWS)
    #include <iphlpapi.h>
    #pragma comment(lib, "iphlpapi.lib")
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <netinet/tcp.h>
    #include <cerrno>
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <net/if.h>
#endif

namespace adam::modules::network
{
    // =========================================================================
    // Bilingual Log-String Tables
    // =========================================================================

    std::string_view port_network::get_error_log_text(socket_error_t err, adam::language lang)
    {
        using arr = std::array<std::string_view, adam::languages_count>;

        static const std::unordered_map<int, arr> table =
        {
            { static_cast<int>(socket_error_t::success),
              { "Success.",
                "Erfolgreich." }},
            { static_cast<int>(socket_error_t::error_address_invalid),
              { "Invalid IP address or hostname.",
                "Ungültige IP-Adresse oder Hostname." }},
            { static_cast<int>(socket_error_t::error_host_unreachable),
              { "Host unreachable.",
                "Host nicht erreichbar." }},
            { static_cast<int>(socket_error_t::error_connection_refused),
              { "Connection refused (port closed or no listener).",
                "Verbindung abgelehnt (Port geschlossen oder kein Listener)." }},
            { static_cast<int>(socket_error_t::error_connection_timeout),
              { "Connection timed out.",
                "Verbindungstimeout überschritten." }},
            { static_cast<int>(socket_error_t::error_connection_lost),
              { "Connection reset or lost.",
                "Verbindung abgebrochen oder verloren." }},
            { static_cast<int>(socket_error_t::error_address_in_use),
              { "Address or port already in use.",
                "Adresse oder Port bereits in Verwendung." }},
            { static_cast<int>(socket_error_t::error_permission_denied),
              { "Permission denied (cannot bind to address/port).",
                "Zugriff verweigert (Bindung an Adresse/Port nicht erlaubt)." }},
            { static_cast<int>(socket_error_t::error_network_down),
              { "Network interface is down.",
                "Netzwerkschnittstelle nicht verfügbar." }},
            { static_cast<int>(socket_error_t::error_unknown),
              { "Unknown network error.",
                "Unbekannter Netzwerkfehler." }},
        };

        auto it = table.find(static_cast<int>(err));
        if (it != table.end())
            return it->second[static_cast<int>(lang)];

        return lang == adam::language_german ? "Unbekannter Fehler." : "Unknown error.";
    }

    std::string_view port_network::get_event_log_text(log_event ev, adam::language lang)
    {
        using arr = std::array<std::string_view, adam::languages_count>;

        static const std::unordered_map<int, arr> table =
        {
            { static_cast<int>(log_event::socket_created),
              { "Socket created successfully.",
                "Socket erfolgreich erstellt." }},
            { static_cast<int>(log_event::socket_creation_failed),
              { "Socket creation failed.",
                "Socket-Erstellung fehlgeschlagen." }},
            { static_cast<int>(log_event::socket_bind_success),
              { "Socket bound successfully.",
                "Socket erfolgreich gebunden." }},
            { static_cast<int>(log_event::socket_bind_failed),
              { "Socket bind failed.",
                "Socket-Bindung fehlgeschlagen." }},
            { static_cast<int>(log_event::socket_listen_success),
              { "Socket listening successfully.",
                "Socket lauscht erfolgreich auf Verbindungen." }},
            { static_cast<int>(log_event::socket_listen_failed),
              { "Socket listen failed.",
                "Socket-Listen fehlgeschlagen." }},
            { static_cast<int>(log_event::socket_connect_success),
              { "Connection established successfully.",
                "Verbindung erfolgreich hergestellt." }},
            { static_cast<int>(log_event::socket_connect_failed),
              { "Connection failed.",
                "Verbindungsaufbau fehlgeschlagen." }},
            { static_cast<int>(log_event::socket_option_failed),
              { "Setting socket option failed.",
                "Festlegen der Socket-Option fehlgeschlagen." }},
            { static_cast<int>(log_event::udp_socket_closed),
              { "UDP socket closed.",
                "UDP-Socket geschlossen." }},
            { static_cast<int>(log_event::udp_send_failed),
              { "UDP send failed.",
                "UDP-Senden fehlgeschlagen." }},
            { static_cast<int>(log_event::multicast_join_success),
              { "Joined multicast group successfully.",
                "Multicast-Gruppe erfolgreich beigetreten." }},
            { static_cast<int>(log_event::multicast_join_failed),
              { "Failed to join multicast group.",
                "Beitritt zur Multicast-Gruppe fehlgeschlagen." }},
            { static_cast<int>(log_event::multicast_address_invalid),
              { "Invalid multicast group address.",
                "Ungültige Multicast-Gruppenadresse." }},
            { static_cast<int>(log_event::tcp_client_connected),
              { "TCP client connected.",
                "TCP-Client verbunden." }},
            { static_cast<int>(log_event::tcp_client_disconnected),
              { "TCP client disconnected.",
                "TCP-Client getrennt." }},
            { static_cast<int>(log_event::tcp_client_stopped),
              { "TCP client connection stopped.",
                "TCP-Client: Verbindung gestoppt." }},
            { static_cast<int>(log_event::tcp_client_send_failed),
              { "TCP client send failed.",
                "TCP-Client: Senden fehlgeschlagen." }},
            { static_cast<int>(log_event::tcp_address_resolution_failed),
              { "Address resolution failed.",
                "Adressauflösung fehlgeschlagen." }},
            { static_cast<int>(log_event::tcp_server_started),
              { "TCP server started.",
                "TCP-Server gestartet." }},
            { static_cast<int>(log_event::tcp_server_stopped),
              { "TCP server stopped, all connections closed.",
                "TCP-Server gestoppt, alle Verbindungen geschlossen." }},
            { static_cast<int>(log_event::tcp_server_client_connected),
              { "New TCP client connected to server.",
                "Neuer TCP-Client am Server verbunden." }},
            { static_cast<int>(log_event::tcp_server_client_disconnected),
              { "TCP client disconnected from server.",
                "TCP-Client vom Server getrennt." }},
            { static_cast<int>(log_event::tcp_server_accept_failed),
              { "Failed to accept new client connection.",
                "Fehler beim Akzeptieren einer neuen Client-Verbindung." }},
            { static_cast<int>(log_event::tcp_server_send_failed),
              { "TCP server send to client failed.",
                "TCP-Server: Senden an Client fehlgeschlagen." }},
        };

        auto it = table.find(static_cast<int>(ev));
        if (it != table.end())
            return it->second[static_cast<int>(lang)];

        return lang == adam::language_german ? "Unbekanntes Netzwerkereignis." : "Unknown network event.";
    }

    // =========================================================================
    // Constructor / Destructor
    // =========================================================================

    port_network::port_network(const string_hashed& item_name, uint32_t state_buffer_size)
        : port_in_out(item_name, state_buffer_size)
        , m_active_ip()
        , m_active_port(0)
    {
        get_parameter<adam::configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());
    }

    std::string port_network::get_active_interface() const
    {
        if (!m_active_interface.empty())
        {
            return m_active_interface;
        }
        return get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct)
            ->get<adam::configuration_parameter_string>("interface"_ct)
            ->get_value();
    }

    void port_network::resolve_active_ip(uintptr_t socket_handle)
    {
        socket_t sock = static_cast<socket_t>(socket_handle);
        if (sock == INVALID_SOCKET_VAL)
        {
            m_active_ip.clear();
            m_active_interface.clear();
            m_active_port = 0;
            return;
        }

        sockaddr_storage local_addr_bound{};
        #if defined(ADAM_PLATFORM_WINDOWS)
        int local_addr_bound_len = sizeof(local_addr_bound);
        #else
        socklen_t local_addr_bound_len = sizeof(local_addr_bound);
        #endif
        if (::getsockname(sock, reinterpret_cast<sockaddr*>(&local_addr_bound), &local_addr_bound_len) == 0)
        {
            char addr_str[INET6_ADDRSTRLEN]{};
            int  local_port = 0;
            if (local_addr_bound.ss_family == AF_INET)
            {
                auto* sa = reinterpret_cast<sockaddr_in*>(&local_addr_bound);
                ::inet_ntop(AF_INET, &sa->sin_addr, addr_str, sizeof(addr_str));
                local_port = ntohs(sa->sin_port);
            }
            else if (local_addr_bound.ss_family == AF_INET6)
            {
                auto* sa = reinterpret_cast<sockaddr_in6*>(&local_addr_bound);
                ::inet_ntop(AF_INET6, &sa->sin6_addr, addr_str, sizeof(addr_str));
                local_port = ntohs(sa->sin6_port);
            }
            m_active_ip = addr_str;
            m_active_port = local_port;

            // Resolve the active interface name from adapter cache
            m_active_interface.clear();
            auto& cache = get_adapter_cache();
            if (cache.empty())
            {
                refresh_adapter_cache();
            }
            for (const auto& info : cache)
            {
                bool found = false;
                for (const auto& ip : info.ipv4_addresses)
                {
                    if (ip == m_active_ip)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    for (const auto& ip : info.ipv6_addresses)
                    {
                        if (ip == m_active_ip)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (found)
                {
                    m_active_interface = !info.friendly_name.empty() ? info.friendly_name : info.adapter_name;
                    break;
                }
            }
        }
    }

    // =========================================================================
    // Logging Wrappers
    // =========================================================================

    void port_network::log_network_message(log::level lvl, log_event ev, std::string_view prefix, const std::string& extra)
    {
        if (auto* ctrl = get_controller())
        {
            language lang = ctrl->get_language();
            std::string prefix_str = prefix.empty() ? "" : std::format("{}: ", prefix);
            std::string extra_str = extra.empty() ? "" : std::format(" {}", extra);
            
            std::string context;
            if (lvl == log::error || lvl == log::warning)
            {
                std::string iface_val = m_active_interface;
                std::string ip_val = m_active_ip;
                int64_t port_val = m_active_port;

                auto user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
                if (user_params)
                {
                    if (iface_val.empty())
                    {
                        if (auto p = user_params->get<adam::configuration_parameter_string>("interface"_ct))
                            iface_val = p->get_value().c_str();
                    }
                    if (port_val <= 0)
                    {
                        if (auto p = user_params->get<adam::configuration_parameter_integer>("interface_port"_ct))
                            port_val = p->get_value();
                    }
                }

                std::vector<std::string> parts;
                if (!iface_val.empty()) parts.push_back(std::format("Interface: {}", iface_val));
                if (!ip_val.empty())
                {
                    if (port_val > 0)
                        parts.push_back(std::format("Local Address: {}:{}", ip_val, port_val));
                    else
                        parts.push_back(std::format("Local IP: {}", ip_val));
                }
                else if (port_val >= 0)
                {
                    parts.push_back(std::format("Local Port: {}", port_val));
                }

                if (!parts.empty())
                {
                    context = " [";
                    for (size_t i = 0; i < parts.size(); ++i)
                    {
                        if (i > 0) context += ", ";
                        context += parts[i];
                    }
                    context += "]";
                }
            }

            std::string_view event_text = get_event_log_text(ev, lang);
            if (ev == log_event::socket_connect_failed && !extra.empty())
            {
                event_text = "";
                if (!extra_str.empty() && extra_str[0] == ' ')
                {
                    extra_str = extra_str.substr(1);
                }
            }

            ctrl->log(lvl, std::format("[{}] {}{}{}{}", get_name().c_str(), prefix_str, event_text, extra_str, context));
        }
    }

    void port_network::log_network_socket_error(log::level lvl, log_event ev, socket_error_t err, std::string_view prefix)
    {
        if (auto* ctrl = get_controller())
        {
            language lang = ctrl->get_language();
            log_network_message(lvl, ev, prefix, std::format("{}", get_error_log_text(err, lang)));
        }
    }

    // =========================================================================
    // OS Error translation static helpers
    // =========================================================================

    int port_network::get_last_error()
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return ::WSAGetLastError();
        #else
        return errno;
        #endif
    }

    void port_network::close_socket(socket_t& s)
    {
        if (s != INVALID_SOCKET_VAL)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            ::closesocket(s);
            #else
            ::close(s);
            #endif
            s = INVALID_SOCKET_VAL;
        }
    }

    void port_network::close_and_clear_socket(socket_t& s)
    {
        close_socket(s);
        resolve_active_ip(INVALID_SOCKET_VAL);
    }

    bool port_network::set_nonblocking(socket_t s, bool non_blocking)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        u_long mode = non_blocking ? 1 : 0;
        return ::ioctlsocket(s, FIONBIO, &mode) == 0;
        #else
        int flags = ::fcntl(s, F_GETFL, 0);
        if (flags == -1) return false;
        flags = non_blocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        return ::fcntl(s, F_SETFL, flags) == 0;
        #endif
    }

    bool port_network::is_blocking_error(int error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS;
        #else
        return error == EWOULDBLOCK || error == EAGAIN || error == EINPROGRESS;
        #endif
    }

    socket_error_t port_network::resolve_socket_error(int os_error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        switch (os_error)
        {
            case 0:                 return socket_error_t::success;
            case WSAEADDRNOTAVAIL:
            case WSAEFAULT:
            case WSAEINVAL:         return socket_error_t::error_address_invalid;
            case WSAEHOSTUNREACH:
            case WSAENETUNREACH:    return socket_error_t::error_host_unreachable;
            case WSAECONNREFUSED:   return socket_error_t::error_connection_refused;
            case WSAETIMEDOUT:      return socket_error_t::error_connection_timeout;
            case WSAECONNRESET:
            case WSAECONNABORTED:
            case WSAESHUTDOWN:
            case WSAENOTCONN:       return socket_error_t::error_connection_lost;
            case WSAEADDRINUSE:     return socket_error_t::error_address_in_use;
            case WSAEACCES:         return socket_error_t::error_permission_denied;
            case WSAENETDOWN:       return socket_error_t::error_network_down;
            default:                return socket_error_t::error_unknown;
        }
        #else
        switch (os_error)
        {
            case 0:                 return socket_error_t::success;
            case EADDRNOTAVAIL:
            case EFAULT:
            case EINVAL:            return socket_error_t::error_address_invalid;
            case EHOSTUNREACH:
            case ENETUNREACH:       return socket_error_t::error_host_unreachable;
            case ECONNREFUSED:      return socket_error_t::error_connection_refused;
            case ETIMEDOUT:         return socket_error_t::error_connection_timeout;
            case ECONNRESET:
            case ECONNABORTED:
            case EPIPE:
            case ENOTCONN:          return socket_error_t::error_connection_lost;
            case EADDRINUSE:        return socket_error_t::error_address_in_use;
            case EACCES:            return socket_error_t::error_permission_denied;
            case ENETDOWN:          return socket_error_t::error_network_down;
            default:                return socket_error_t::error_unknown;
        }
        #endif
    }

    // =========================================================================
    // Address Resolution
    // =========================================================================

    bool port_network::resolve_address(const adam::string_hashed& host, int port,
                                        const adam::string_hashed& ip_version,
                                        sockaddr_storage& out_addr, int& out_addr_len, int sock_type)
    {
        std::memset(&out_addr, 0, sizeof(out_addr));
        out_addr_len = 0;

        addrinfo hints{};
        hints.ai_socktype = sock_type;

        if      (ip_version == "ipv4"_ct) hints.ai_family = AF_INET;
        else if (ip_version == "ipv6"_ct) hints.ai_family = AF_INET6;
        else                              hints.ai_family = AF_UNSPEC;

        const bool wildcard = host.empty() || host == "auto"_ct;
        if (wildcard) hints.ai_flags = AI_PASSIVE;

        const std::string port_str = std::to_string(port);
        const char*       host_ptr = wildcard ? nullptr : host.c_str();

        addrinfo* res = nullptr;
        if (::getaddrinfo(host_ptr, port_str.c_str(), &hints, &res) != 0 || !res)
            return false;

        std::memcpy(&out_addr, res->ai_addr, res->ai_addrlen);
        out_addr_len = static_cast<int>(res->ai_addrlen);
        ::freeaddrinfo(res);
        return true;
    }

    void port_network::apply_dual_stack_if_auto(socket_t sock, int family,
                                                 const adam::string_hashed& ip_ver)
    {
        if (family != AF_INET6 || ip_ver != "auto"_ct)
            return;

        #if defined(ADAM_PLATFORM_WINDOWS)
        DWORD ipv6_only = 0;
        #else
        int   ipv6_only = 0;
        #endif
        ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                     reinterpret_cast<const char*>(&ipv6_only), sizeof(ipv6_only));
    }

    // =========================================================================
    // Interface Enumeration & Routing
    // =========================================================================

    std::vector<adapter_info>& port_network::get_adapter_cache()
    {
        static std::vector<adapter_info> cache;
        return cache;
    }

    #if defined(ADAM_PLATFORM_WINDOWS)
    static std::string pwchar_to_utf8(PWCHAR wstr)
    {
        if (!wstr) return "";
        int size_needed = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) return "";
        std::string str(size_needed, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);
        if (!str.empty() && str.back() == '\0') str.pop_back();
        return str;
    }

    static bool is_virtual_or_useless_adapter(PIP_ADAPTER_ADDRESSES adapter)
    {
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK || adapter->IfType == IF_TYPE_TUNNEL)
        {
            return true;
        }
        if (adapter->IfType == 71 && adapter->Flags != 485)
        {
            return true;
        }
        return false;
    }
    #endif

    void port_network::refresh_adapter_cache()
    {
        auto& cache = get_adapter_cache();
        cache.clear();

        #if defined(ADAM_PLATFORM_WINDOWS)
        ULONG out_buf_len = 15000;
        PIP_ADAPTER_ADDRESSES p_addresses = (PIP_ADAPTER_ADDRESSES)std::malloc(out_buf_len);
        if (p_addresses)
        {
            ULONG flags = GAA_FLAG_SKIP_DNS_SERVER;
            DWORD dw_ret_val = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, p_addresses, &out_buf_len);
            if (dw_ret_val == ERROR_BUFFER_OVERFLOW)
            {
                std::free(p_addresses);
                p_addresses = (PIP_ADAPTER_ADDRESSES)std::malloc(out_buf_len);
            }
            if (p_addresses && GetAdaptersAddresses(AF_UNSPEC, flags, NULL, p_addresses, &out_buf_len) == NO_ERROR)
            {
                for (PIP_ADAPTER_ADDRESSES p_curr_addresses = p_addresses; p_curr_addresses != nullptr; p_curr_addresses = p_curr_addresses->Next)
                {
                    if (is_virtual_or_useless_adapter(p_curr_addresses))
                    {
                        continue;
                    }

                    adapter_info info;
                    if (p_curr_addresses->FriendlyName)
                    {
                        info.friendly_name = pwchar_to_utf8(p_curr_addresses->FriendlyName);
                    }
                    if (p_curr_addresses->AdapterName)
                    {
                        info.adapter_name = p_curr_addresses->AdapterName;
                    }
                    info.index = p_curr_addresses->Ipv6IfIndex ? p_curr_addresses->Ipv6IfIndex : p_curr_addresses->IfIndex;

                    for (PIP_ADAPTER_UNICAST_ADDRESS p_unicast = p_curr_addresses->FirstUnicastAddress; p_unicast != nullptr; p_unicast = p_unicast->Next)
                    {
                        if (p_unicast->Address.lpSockaddr)
                        {
                            char addr_str[INET6_ADDRSTRLEN]{};
                            if (p_unicast->Address.lpSockaddr->sa_family == AF_INET)
                            {
                                auto* sa = reinterpret_cast<sockaddr_in*>(p_unicast->Address.lpSockaddr);
                                if (::inet_ntop(AF_INET, &sa->sin_addr, addr_str, sizeof(addr_str)))
                                {
                                    info.ipv4_addresses.push_back(addr_str);
                                }
                            }
                            else if (p_unicast->Address.lpSockaddr->sa_family == AF_INET6)
                            {
                                auto* sa = reinterpret_cast<sockaddr_in6*>(p_unicast->Address.lpSockaddr);
                                if (::inet_ntop(AF_INET6, &sa->sin6_addr, addr_str, sizeof(addr_str)))
                                {
                                    info.ipv6_addresses.push_back(addr_str);
                                }
                            }
                        }
                    }
                    cache.push_back(info);
                }
            }
            if (p_addresses) std::free(p_addresses);
        }
        #else
        struct ifaddrs* ifap = nullptr;
        if (::getifaddrs(&ifap) == 0)
        {
            std::unordered_map<std::string, adapter_info> map;
            for (struct ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next)
            {
                if (!ifa->ifa_name || std::strlen(ifa->ifa_name) == 0 || !ifa->ifa_addr)
                {
                    continue;
                }
                std::string name(ifa->ifa_name);
                if (name == "lo" || name.find("loopback") != std::string::npos)
                {
                    continue;
                }

                auto& info = map[name];
                info.friendly_name = name;
                info.adapter_name = name;
                info.index = ::if_nametoindex(ifa->ifa_name);

                char addr_str[INET6_ADDRSTRLEN]{};
                if (ifa->ifa_addr->sa_family == AF_INET)
                {
                    auto* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
                    if (::inet_ntop(AF_INET, &sa->sin_addr, addr_str, sizeof(addr_str)))
                    {
                        info.ipv4_addresses.push_back(addr_str);
                    }
                }
                else if (ifa->ifa_addr->sa_family == AF_INET6)
                {
                    auto* sa = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
                    if (::inet_ntop(AF_INET6, &sa->sin6_addr, addr_str, sizeof(addr_str)))
                    {
                        info.ipv6_addresses.push_back(addr_str);
                    }
                }
            }
            ::freeifaddrs(ifap);

            for (auto& [name, info] : map)
            {
                cache.push_back(info);
            }
        }
        #endif
    }

    std::vector<std::string> port_network::get_system_interfaces()
    {
        refresh_adapter_cache();
        std::vector<std::string> interfaces;
        for (const auto& info : get_adapter_cache())
        {
            if (!info.friendly_name.empty())
            {
                interfaces.push_back(info.friendly_name);
            }
            else if (!info.adapter_name.empty())
            {
                interfaces.push_back(info.adapter_name);
            }
        }
        std::sort(interfaces.begin(), interfaces.end());
        interfaces.erase(std::unique(interfaces.begin(), interfaces.end()), interfaces.end());
        return interfaces;
    }

    std::string port_network::resolve_auto_interface_for_remote(const adam::string_hashed& remote_ip, int remote_port, const adam::string_hashed& ip_version)
    {
        if (remote_ip.empty())
            return "";

        sockaddr_storage dest_addr{};
        int dest_addr_len = 0;
        if (!resolve_address(remote_ip, remote_port, ip_version, dest_addr, dest_addr_len, SOCK_DGRAM))
        {
            return "";
        }

        socket_t temp_sock = ::socket(dest_addr.ss_family, SOCK_DGRAM, IPPROTO_UDP);
        if (temp_sock == INVALID_SOCKET_VAL)
        {
            return "";
        }

        if (::connect(temp_sock, reinterpret_cast<sockaddr*>(&dest_addr), dest_addr_len) == SOCKET_ERROR_VAL)
        {
            close_socket(temp_sock);
            return "";
        }

        sockaddr_storage local_addr{};
        #if defined(ADAM_PLATFORM_WINDOWS)
        int local_addr_len = sizeof(local_addr);
        #else
        socklen_t local_addr_len = sizeof(local_addr);
        #endif

        if (::getsockname(temp_sock, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_len) == SOCKET_ERROR_VAL)
        {
            close_socket(temp_sock);
            return "";
        }

        close_socket(temp_sock);

        char local_ip_str[INET6_ADDRSTRLEN]{};
        if (local_addr.ss_family == AF_INET)
        {
            auto* sa = reinterpret_cast<sockaddr_in*>(&local_addr);
            if (::inet_ntop(AF_INET, &sa->sin_addr, local_ip_str, sizeof(local_ip_str)))
            {
                return local_ip_str;
            }
        }
        else if (local_addr.ss_family == AF_INET6)
        {
            auto* sa = reinterpret_cast<sockaddr_in6*>(&local_addr);
            if (::inet_ntop(AF_INET6, &sa->sin6_addr, local_ip_str, sizeof(local_ip_str)))
            {
                return local_ip_str;
            }
        }

        return "";
    }

    std::string port_network::resolve_interface_to_ip(const adam::string_hashed& interface_name, int family)
    {
        if (interface_name.empty() || interface_name == "auto"_ct)
        {
            return "";
        }

        auto& cache = get_adapter_cache();
        if (cache.empty())
        {
            refresh_adapter_cache();
        }

        for (const auto& info : cache)
        {
            if (info.friendly_name == interface_name || info.adapter_name == interface_name)
            {
                if (family == AF_INET && !info.ipv4_addresses.empty())
                {
                    return info.ipv4_addresses.front();
                }
                if (family == AF_INET6 && !info.ipv6_addresses.empty())
                {
                    return info.ipv6_addresses.front();
                }
                if (family == AF_UNSPEC)
                {
                    return !info.ipv4_addresses.empty() ? info.ipv4_addresses.front() : (!info.ipv6_addresses.empty() ? info.ipv6_addresses.front() : "");
                }
            }
        }
        return "";
    }

    bool port_network::resolve_local_interface_to_ip(const adam::string_hashed& interface_val,
                                                     const adam::string_hashed& remote_ip,
                                                     int remote_port,
                                                     const adam::string_hashed& ip_version,
                                                     std::string& resolved_ip)
    {
        int family = AF_UNSPEC;
        if (ip_version == "ipv4"_ct) family = AF_INET;
        else if (ip_version == "ipv6"_ct) family = AF_INET6;
        else if (!remote_ip.empty())
        {
            sockaddr_storage dest_addr{};
            int dest_addr_len = 0;
            if (resolve_address(remote_ip, remote_port, ip_version, dest_addr, dest_addr_len, SOCK_DGRAM))
            {
                family = dest_addr.ss_family;
            }
        }

        if (interface_val.empty() || interface_val == "auto"_ct)
        {
            if (!remote_ip.empty())
            {
                resolved_ip = resolve_auto_interface_for_remote(remote_ip, remote_port, ip_version);
                if (!resolved_ip.empty())
                {
                    return true;
                }
            }
            resolved_ip = (family == AF_INET6) ? "::" : "0.0.0.0";
            return true;
        }

        resolved_ip = resolve_interface_to_ip(interface_val, family);
        if (!resolved_ip.empty())
        {
            return true;
        }

        if (family != AF_UNSPEC)
        {
            resolved_ip = resolve_interface_to_ip(interface_val, AF_UNSPEC);
            if (!resolved_ip.empty())
            {
                return true;
            }
        }

        return false;
    }

    unsigned int port_network::get_interface_index(const adam::string_hashed& interface_val,
                                                   const adam::string_hashed& dest_ip,
                                                   int dest_port,
                                                   const adam::string_hashed& ip_version)
    {
        if (interface_val.empty() || interface_val == "auto"_ct)
        {
            if (!dest_ip.empty())
            {
                std::string local_ip = resolve_auto_interface_for_remote(dest_ip, dest_port, ip_version);
                if (!local_ip.empty())
                {
                    auto& cache = get_adapter_cache();
                    if (cache.empty()) refresh_adapter_cache();
                    for (const auto& info : cache)
                    {
                        for (const auto& ip : info.ipv4_addresses)
                        {
                            if (ip == local_ip) return info.index;
                        }
                        for (const auto& ip : info.ipv6_addresses)
                        {
                            if (ip == local_ip) return info.index;
                        }
                    }
                }
            }
            return 0;
        }

        auto& cache = get_adapter_cache();
        if (cache.empty()) refresh_adapter_cache();
        for (const auto& info : cache)
        {
            if (info.friendly_name == interface_val || info.adapter_name == interface_val)
            {
                return info.index;
            }
        }

        #if defined(ADAM_PLATFORM_WINDOWS)
        return 0;
        #else
        return ::if_nametoindex(interface_val.c_str());
        #endif
    }

    std::unique_ptr<adam::configuration_parameter_string> port_network::create_interface_parameter()
    {
        configuration_parameter_string::presets_container presets;
        presets.emplace("auto"_ct, std::make_unique<configuration_parameter_string>("auto"_ct, "auto"_ct));

        for (const auto& name : get_system_interfaces())
        {
            string_hashed hashed_name(name.c_str());
            presets.emplace(hashed_name, std::make_unique<configuration_parameter_string>(hashed_name, hashed_name));
        }

        return std::make_unique<adam::configuration_parameter_string>(
            "interface"_ct, "auto"_ct, std::move(presets));
    }

    std::unique_ptr<adam::configuration_parameter_string> port_network::create_ip_regex_parameter()
    {
        return std::make_unique<adam::configuration_parameter_string>(
            "regex"_ct, "^$|^auto$|^[a-zA-Z0-9.:-]+$"_ct);
    }

    // =========================================================================
    // Consolidated Address Bind
    // =========================================================================

    bool port_network::resolve_bind_address(const adam::string_hashed& interface_val,
                                            const adam::string_hashed& remote_ip,
                                            int remote_port,
                                            int local_port,
                                            const adam::string_hashed& ip_version,
                                            int sock_type,
                                            sockaddr_storage& out_addr,
                                            int& out_addr_len,
                                            std::string& resolved_ip)
    {
        if (!resolve_local_interface_to_ip(interface_val, remote_ip, remote_port, ip_version, resolved_ip))
        {
            return false;
        }
        return resolve_address(adam::string_hashed(resolved_ip.c_str()), local_port, ip_version, out_addr, out_addr_len, sock_type);
    }

    // =========================================================================
    // Robust Send Helpers
    // =========================================================================

    bool port_network::send_all(socket_t sock, const char* data, size_t size)
    {
        size_t total_sent = 0;
        while (total_sent < size && is_running())
        {
            if (sock == INVALID_SOCKET_VAL) return false;

            #if defined(ADAM_PLATFORM_WINDOWS)
            int sent = ::send(sock, data + total_sent, static_cast<int>(size - total_sent), 0);
            #else
            int sent = ::send(sock, data + total_sent, size - total_sent, 0);
            #endif

            if (sent < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err))
                {
                    // Wait for writability (timeout 10ms)
                    fd_set write_fds;
                    FD_ZERO(&write_fds);
                    FD_SET(sock, &write_fds);
                    timeval tv{ 0, 10000 }; // 10ms
                    int rv = ::select(static_cast<int>(sock) + 1, nullptr, &write_fds, nullptr, &tv);
                    if (rv > 0)
                    {
                        continue; // Writable, retry
                    }
                    if (rv < 0)
                    {
                        return false;
                    }
                    continue; // Timeout, loop and retry if running
                }
                return false;
            }
            if (sent == 0)
            {
                return false;
            }
            total_sent += sent;
        }
        return total_sent == size;
    }

    bool port_network::send_all_nonblocking(socket_t sock, const char* data, size_t size)
    {
        size_t total_sent = 0;
        int retries = 0;
        while (total_sent < size)
        {
            if (sock == INVALID_SOCKET_VAL) return false;

            #if defined(ADAM_PLATFORM_WINDOWS)
            int sent = ::send(sock, data + total_sent, static_cast<int>(size - total_sent), 0);
            #else
            int sent = ::send(sock, data + total_sent, size - total_sent, 0);
            #endif

            if (sent < 0)
            {
                int err = get_last_error();
                if (is_blocking_error(err))
                {
                    if (++retries > 5) return false; // Too many would-block retries, fail connection
                    std::this_thread::yield();
                    continue;
                }
                return false;
            }
            if (sent == 0) return false;
            total_sent += sent;
            retries = 0;
        }
        return true;
    }
}
