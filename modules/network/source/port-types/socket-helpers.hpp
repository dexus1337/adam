#pragma once

/**
 * @file    socket-helpers.hpp
 * @author  dexus1337
 * @brief   Platform-independent socket types, helpers, and bilingual log-string tables
 *          for all network port implementations.
 *
 *          All user-visible strings are stored in static unordered_map tables indexed
 *          by enum value and language, matching the SDK language-strings.cpp pattern.
 *          No hardcoded inline strings should appear in any port .cpp file.
 *
 * @version 2.0
 * @date    04.07.2026
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
    #define SOCKET_ERROR_VAL   SOCKET_ERROR
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
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL   (-1)
#endif

#include <array>
#include <unordered_map>
#include <string_view>
#include <string>
#include <cstring>

namespace adam::modules::network
{
    // =========================================================================
    // Enums
    // =========================================================================

    /**
     * @enum  socket_error_t
     * @brief Semantic error categories derived from platform socket error codes.
     *        Use @ref resolve_socket_error to convert OS error integers into these values.
     */
    enum class socket_error_t
    {
        success = 0,            ///< No error.
        error_address_invalid,  ///< The supplied IP address or hostname is invalid.
        error_host_unreachable, ///< The target host cannot be reached.
        error_connection_refused, ///< The remote end actively refused the connection.
        error_connection_timeout, ///< The connection attempt timed out.
        error_connection_lost,  ///< An established connection was reset or closed unexpectedly.
        error_address_in_use,   ///< The local address or port is already bound.
        error_permission_denied,///< Insufficient privileges to bind or connect.
        error_network_down,     ///< The local network interface is down.
        error_unknown           ///< An unrecognised OS error code was encountered.
    };

    /**
     * @enum  log_event
     * @brief Named log events used by all network port types.
     *        Pass to @ref get_event_log_text to retrieve a bilingual message string.
     */
    enum class log_event
    {
        // --- Generic socket lifecycle ---
        socket_created,                  ///< A socket was successfully created.
        socket_creation_failed,          ///< Socket creation (::socket()) failed.
        socket_bind_success,             ///< Socket was successfully bound to a local address/port.
        socket_bind_failed,              ///< Socket bind failed.
        socket_listen_success,           ///< Server socket began listening successfully.
        socket_listen_failed,            ///< Server socket listen failed.
        socket_connect_success,          ///< TCP connection established successfully.
        socket_connect_failed,           ///< TCP connection attempt failed.
        socket_option_failed,            ///< A setsockopt/ioctlsocket call failed.

        // --- UDP ---
        udp_socket_closed,               ///< UDP socket was closed cleanly.
        udp_send_failed,                 ///< A UDP sendto() call failed.

        // --- Multicast ---
        multicast_join_success,          ///< Successfully joined a multicast group.
        multicast_join_failed,           ///< Failed to join a multicast group.
        multicast_address_invalid,       ///< The multicast group address string was invalid.

        // --- TCP client ---
        tcp_client_connected,            ///< TCP client established a connection to the server.
        tcp_client_disconnected,         ///< TCP client connection was lost or closed by server.
        tcp_client_stopped,              ///< TCP client was explicitly stopped.
        tcp_client_send_failed,          ///< A TCP client send() call failed.
        tcp_address_resolution_failed,   ///< DNS/address resolution failed before connecting.

        // --- TCP server ---
        tcp_server_started,              ///< TCP server is listening.
        tcp_server_stopped,              ///< TCP server listener and all client sockets closed.
        tcp_server_client_connected,     ///< A new client connected to the TCP server.
        tcp_server_client_disconnected,  ///< A client disconnected from the TCP server.
        tcp_server_accept_failed,        ///< accept() on the listener socket failed.
        tcp_server_send_failed           ///< A TCP server send() call to a client failed.
    };

    // =========================================================================
    // Platform helpers
    // =========================================================================

    /**
     * @brief Returns the last socket error code for the current thread.
     *        Maps to WSAGetLastError() on Windows and errno on POSIX.
     */
    inline int get_last_error()
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return ::WSAGetLastError();
        #else
        return errno;
        #endif
    }

    /**
     * @brief Closes a socket handle and resets it to INVALID_SOCKET_VAL.
     * @param s  Reference to the socket handle to close.
     */
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

    /**
     * @brief Puts a socket into blocking or non-blocking mode.
     * @param s           The socket to configure.
     * @param nonblocking If true, enables non-blocking mode.
     * @return True on success, false on error.
     */
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

    /**
     * @brief Returns true if the OS error code represents a non-fatal "would block" condition.
     *        These should be silently retried rather than treated as failures.
     */
    inline bool is_blocking_error(int error)
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS;
        #else
        return error == EWOULDBLOCK || error == EAGAIN || error == EINPROGRESS;
        #endif
    }

    /**
     * @brief Maps a raw OS socket error code to a semantic @ref socket_error_t value.
     * @param os_error  The raw OS error integer (from @ref get_last_error).
     * @return The closest matching @ref socket_error_t.
     */
    inline socket_error_t resolve_socket_error(int os_error)
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
    // Bilingual string tables
    // =========================================================================

    /**
     * @brief Returns a bilingual human-readable description for a @ref socket_error_t value.
     *
     *        Strings are stored in a static unordered_map keyed by enum value, with one
     *        std::array<std::string_view, languages_count> per entry (index 0 = English,
     *        index 1 = German), matching the SDK language-strings.cpp pattern.
     *
     * @param err   The semantic error to describe.
     * @param lang  The desired output language.
     * @return A string_view into static storage — valid for the lifetime of the process.
     */
    inline std::string_view get_error_log_text(socket_error_t err, adam::language lang)
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

    /**
     * @brief Returns a bilingual human-readable description for a @ref log_event value.
     *
     *        Strings are stored in a static unordered_map keyed by enum value, with one
     *        std::array<std::string_view, languages_count> per entry (index 0 = English,
     *        index 1 = German), matching the SDK language-strings.cpp pattern.
     *
     * @param ev    The log event to describe.
     * @param lang  The desired output language.
     * @return A string_view into static storage — valid for the lifetime of the process.
     */
    inline std::string_view get_event_log_text(log_event ev, adam::language lang)
    {
        using arr = std::array<std::string_view, adam::languages_count>;

        static const std::unordered_map<int, arr> table =
        {
            // --- Generic socket lifecycle ---
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

            // --- UDP ---
            { static_cast<int>(log_event::udp_socket_closed),
              { "UDP socket closed.",
                "UDP-Socket geschlossen." }},
            { static_cast<int>(log_event::udp_send_failed),
              { "UDP send failed.",
                "UDP-Senden fehlgeschlagen." }},

            // --- Multicast ---
            { static_cast<int>(log_event::multicast_join_success),
              { "Joined multicast group successfully.",
                "Multicast-Gruppe erfolgreich beigetreten." }},
            { static_cast<int>(log_event::multicast_join_failed),
              { "Failed to join multicast group.",
                "Beitritt zur Multicast-Gruppe fehlgeschlagen." }},
            { static_cast<int>(log_event::multicast_address_invalid),
              { "Invalid multicast group address.",
                "Ungültige Multicast-Gruppenadresse." }},

            // --- TCP client ---
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

            // --- TCP server ---
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
    // Address resolution
    // =========================================================================

    inline std::unique_ptr<adam::configuration_parameter_string> create_ip_regex_parameter()
    {
        return std::make_unique<adam::configuration_parameter_string>(
            "regex"_ct, "^$|^auto$|^[a-zA-Z0-9.:-]+$"_ct);
    }


    /**
     * @brief Resolves a host and port to a sockaddr_storage using getaddrinfo.
     *
     *        Accepts a @ref adam::string_hashed so callers can pass the value directly
     *        from a configuration_parameter_string::get_value() without constructing a
     *        temporary std::string.  Comparisons against literals use the O(1) hash path
     *        via string_hashed::operator==(string_hashed_ct).
     *
     * @param host          Hostname or IP as a string_hashed.  An empty string or "auto"
     *                      means "bind to any address" (AI_PASSIVE); no special sentinel
     *                      values like "0.0.0.0" or "::" are needed or accepted.
     * @param port          Port number (0 for any/ephemeral).
     * @param ip_version    string_hashed containing "ipv4", "ipv6", or anything else for
     *                      auto (AF_UNSPEC).  Compared by hash — O(1).
     * @param out_addr      Output: filled sockaddr_storage on success.
     * @param out_addr_len  Output: byte length of the filled sockaddr on success.
     * @param sock_type     SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
     * @return True if address resolution succeeded, false otherwise.
     */
    inline bool resolve_address(const adam::string_hashed& host, int port,
                                const adam::string_hashed& ip_version,
                                sockaddr_storage& out_addr, int& out_addr_len, int sock_type)
    {
        std::memset(&out_addr, 0, sizeof(out_addr));
        out_addr_len = 0;

        addrinfo hints{};
        hints.ai_socktype = sock_type;

        // Hash comparisons — O(1), no character-by-character scan.
        if      (ip_version == "ipv4"_ct) hints.ai_family = AF_INET;
        else if (ip_version == "ipv6"_ct) hints.ai_family = AF_INET6;
        else                              hints.ai_family = AF_UNSPEC;

        // Empty host or "auto" means "any local address" — use AI_PASSIVE so getaddrinfo fills
        // the wildcard address for the chosen family (INADDR_ANY / in6addr_any).
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

    /**
     * @brief Applies IPV6_V6ONLY = 0 (dual-stack) when ip_version is "auto" and the
     *        resolved address family is AF_INET6.  No-op in all other cases.
     *
     * @param sock      The socket to configure.
     * @param family    The address family resolved by getaddrinfo (ss_family).
     * @param ip_ver    string_hashed containing the ip_version value.  Compared by hash.
     */
    inline void apply_dual_stack_if_auto(socket_t sock, int family,
                                         const adam::string_hashed& ip_ver)
    {
        // Hash comparison — O(1).
        if (family != AF_INET6 || ip_ver != "auto"_ct)
            return;

        #if defined(ADAM_PLATFORM_WINDOWS)
        DWORD ipv6only = 0;
        #else
        int   ipv6only = 0;
        #endif
        ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                     reinterpret_cast<const char*>(&ipv6only), sizeof(ipv6only));
    }

} // namespace adam::modules::network

