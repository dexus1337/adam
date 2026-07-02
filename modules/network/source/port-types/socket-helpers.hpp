#pragma once

/**
 * @file    socket-helpers.hpp
 * @author  dexus1337
 * @brief   Provides platform-independent types, functions, and semantic error mappings for socket operations.
 * @version 1.0
 * @date    02.07.2026
 */

#include <adam-sdk.hpp>

#if defined(ADAM_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <netinet/tcp.h>
    #include <cerrno>
    #include <netdb.h>
    using socket_t = int;
    #define INVALID_SOCKET_VAL -1
    #define SOCKET_ERROR_VAL -1
#endif

#include <string_view>
#include <string>
#include <cstring>

namespace adam::modules::network
{
    enum class socket_error_t
    {
        success = 0,
        error_address_invalid,
        error_host_unreachable,
        error_connection_refused,
        error_connection_timeout,
        error_connection_lost,
        error_address_in_use,
        error_permission_denied,
        error_network_down,
        error_unknown
    };

    enum class log_event
    {
        socket_created,
        socket_creation_failed,
        socket_bind_success,
        socket_bind_failed,
        socket_listen_success,
        socket_listen_failed,
        socket_connect_success,
        socket_connect_failed,
        socket_option_failed,
        multicast_join_success,
        multicast_join_failed,
        tcp_client_connected,
        tcp_client_disconnected,
        tcp_server_client_connected,
        tcp_server_client_disconnected,
        tcp_server_accept_failed
    };

    inline int get_last_error()
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return ::WSAGetLastError();
        #else
        return errno;
        #endif
    }

    inline void close_socket(socket_t& s)
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

    inline bool set_nonblocking(socket_t s, bool nonblocking)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        u_long mode = nonblocking ? 1 : 0;
        return ::ioctlsocket(s, FIONBIO, &mode) == 0;
        #else
        int flags = ::fcntl(s, F_GETFL, 0);
        if (flags == -1) return false;
        flags = nonblocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        return ::fcntl(s, F_SETFL, flags) == 0;
        #endif
    }

    inline bool is_blocking_error(int error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS;
        #else
        return error == EWOULDBLOCK || error == EAGAIN || error == EINPROGRESS;
        #endif
    }

    inline socket_error_t resolve_socket_error(int os_error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        switch (os_error)
        {
            case 0:                      return socket_error_t::success;
            case WSAEADDRNOTAVAIL:
            case WSAEFAULT:
            case WSAEINVAL:              return socket_error_t::error_address_invalid;
            case WSAEHOSTUNREACH:
            case WSAENETUNREACH:         return socket_error_t::error_host_unreachable;
            case WSAECONNREFUSED:        return socket_error_t::error_connection_refused;
            case WSAETIMEDOUT:           return socket_error_t::error_connection_timeout;
            case WSAECONNRESET:
            case WSAECONNABORTED:
            case WSAESHUTDOWN:
            case WSAENOTCONN:            return socket_error_t::error_connection_lost;
            case WSAEADDRINUSE:          return socket_error_t::error_address_in_use;
            case WSAEACCES:              return socket_error_t::error_permission_denied;
            case WSAENETDOWN:            return socket_error_t::error_network_down;
            default:                     return socket_error_t::error_unknown;
        }
        #else
        switch (os_error)
        {
            case 0:                      return socket_error_t::success;
            case EADDRNOTAVAIL:
            case EFAULT:
            case EINVAL:                 return socket_error_t::error_address_invalid;
            case EHOSTUNREACH:
            case ENETUNREACH:            return socket_error_t::error_host_unreachable;
            case ECONNREFUSED:           return socket_error_t::error_connection_refused;
            case ETIMEDOUT:              return socket_error_t::error_connection_timeout;
            case ECONNRESET:
            case ECONNABORTED:
            case EPIPE:
            case ENOTCONN:               return socket_error_t::error_connection_lost;
            case EADDRINUSE:             return socket_error_t::error_address_in_use;
            case EACCES:                 return socket_error_t::error_permission_denied;
            case ENETDOWN:               return socket_error_t::error_network_down;
            default:                     return socket_error_t::error_unknown;
        }
        #endif
    }

    inline std::string_view get_error_log_text(socket_error_t err, language lang)
    {
        if (lang == language_german)
        {
            switch (err)
            {
                case socket_error_t::success:                  return "Erfolg";
                case socket_error_t::error_address_invalid:    return "Ungültige IP-Adresse oder Hostname";
                case socket_error_t::error_host_unreachable:   return "Host nicht erreichbar";
                case socket_error_t::error_connection_refused: return "Verbindung abgelehnt (Port geschlossen oder kein Listener)";
                case socket_error_t::error_connection_timeout: return "Verbindungstimeout überschritten";
                case socket_error_t::error_connection_lost:    return "Verbindung abgebrochen oder verloren";
                case socket_error_t::error_address_in_use:     return "Adresse/Port bereits in Verwendung";
                case socket_error_t::error_permission_denied:  return "Zugriff verweigert (Berechtigungsproblem bei Portbindung)";
                case socket_error_t::error_network_down:       return "Netzwerk nicht verfügbar";
                default:                                       return "Unbekannter Netzwerkfehler";
            }
        }
        else
        {
            switch (err)
            {
                case socket_error_t::success:                  return "Success";
                case socket_error_t::error_address_invalid:    return "Invalid IP address or hostname";
                case socket_error_t::error_host_unreachable:   return "Host unreachable";
                case socket_error_t::error_connection_refused: return "Connection refused (port closed or no listener)";
                case socket_error_t::error_connection_timeout: return "Connection timeout expired";
                case socket_error_t::error_connection_lost:    return "Connection reset or lost";
                case socket_error_t::error_address_in_use:     return "Address or port already in use";
                case socket_error_t::error_permission_denied:  return "Permission denied (cannot bind to port)";
                case socket_error_t::error_network_down:       return "Network down";
                default:                                       return "Unknown network error";
            }
        }
    }

    inline std::string_view get_event_log_text(log_event ev, language lang)
    {
        if (lang == language_german)
        {
            switch (ev)
            {
                case log_event::socket_created:                return "Socket erfolgreich erstellt.";
                case log_event::socket_creation_failed:        return "Socket-Erstellung fehlgeschlagen.";
                case log_event::socket_bind_success:           return "Socket erfolgreich an Schnittstelle gebunden.";
                case log_event::socket_bind_failed:            return "Socket-Bindung fehlgeschlagen.";
                case log_event::socket_listen_success:         return "Socket lauscht erfolgreich auf Verbindungen.";
                case log_event::socket_listen_failed:          return "Socket-Listen fehlgeschlagen.";
                case log_event::socket_connect_success:        return "Verbindung erfolgreich hergestellt.";
                case log_event::socket_connect_failed:         return "Verbindungsaufbau fehlgeschlagen.";
                case log_event::socket_option_failed:          return "Festlegen der Socket-Option fehlgeschlagen.";
                case log_event::multicast_join_success:        return "Multicast-Gruppe erfolgreich beigetreten.";
                case log_event::multicast_join_failed:         return "Beitritt zur Multicast-Gruppe fehlgeschlagen.";
                case log_event::tcp_client_connected:          return "TCP-Client verbunden.";
                case log_event::tcp_client_disconnected:       return "TCP-Client getrennt.";
                case log_event::tcp_server_client_connected:   return "Neuer TCP-Client am Server verbunden.";
                case log_event::tcp_server_client_disconnected: return "TCP-Client vom Server getrennt.";
                case log_event::tcp_server_accept_failed:      return "Fehler beim Akzeptieren einer neuen Client-Verbindung.";
                default:                                       return "Unbekanntes Netzwerkereignis.";
            }
        }
        else
        {
            switch (ev)
            {
                case log_event::socket_created:                return "Socket created successfully.";
                case log_event::socket_creation_failed:        return "Socket creation failed.";
                case log_event::socket_bind_success:           return "Socket bound successfully.";
                case log_event::socket_bind_failed:            return "Socket bind failed.";
                case log_event::socket_listen_success:         return "Socket listening successfully.";
                case log_event::socket_listen_failed:          return "Socket listen failed.";
                case log_event::socket_connect_success:        return "Connection established successfully.";
                case log_event::socket_connect_failed:         return "Connection failed.";
                case log_event::socket_option_failed:          return "Setting socket option failed.";
                case log_event::multicast_join_success:        return "Joined multicast group successfully.";
                case log_event::multicast_join_failed:         return "Failed to join multicast group.";
                case log_event::tcp_client_connected:          return "TCP client connected.";
                case log_event::tcp_client_disconnected:       return "TCP client disconnected.";
                case log_event::tcp_server_client_connected:   return "New TCP client connected to server.";
                case log_event::tcp_server_client_disconnected: return "TCP client disconnected from server.";
                case log_event::tcp_server_accept_failed:      return "Failed to accept new client connection.";
                default:                                       return "Unknown network event.";
            }
        }
    }

    inline bool resolve_address(const std::string& host, int port, const std::string& ip_version, sockaddr_storage& out_addr, int& out_addr_len, int sock_type)
    {
        std::memset(&out_addr, 0, sizeof(out_addr));
        out_addr_len = 0;

        addrinfo hints;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = sock_type;

        if (ip_version == "ipv4")
        {
            hints.ai_family = AF_INET;
        }
        else if (ip_version == "ipv6")
        {
            hints.ai_family = AF_INET6;
        }
        else // auto
        {
            hints.ai_family = AF_UNSPEC;
        }

        if (host.empty() || host == "0.0.0.0" || host == "::")
        {
            hints.ai_flags = AI_PASSIVE;
        }

        std::string port_str = std::to_string(port);
        addrinfo* res = nullptr;
        const char* host_ptr = host.empty() ? nullptr : host.c_str();
        
        int status = ::getaddrinfo(host_ptr, port_str.c_str(), &hints, &res);
        if (status != 0 || !res)
        {
            return false;
        }

        std::memcpy(&out_addr, res->ai_addr, res->ai_addrlen);
        out_addr_len = static_cast<int>(res->ai_addrlen);
        ::freeaddrinfo(res);
        return true;
    }
}
