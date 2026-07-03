#pragma once

/**
 * @file    port-tcp-server.hpp
 * @author  dexus1337
 * @brief   Declares the TCP Server Port for accepting multiple simultaneous client connections.
 *
 *          The port runs a single background worker thread that:
 *            1. Accepts incoming TCP client connections on a non-blocking listener socket.
 *            2. Reads from all connected clients in a single select() call.
 *            3. Tracks connected clients in m_clients (protected by m_clients_mutex spinlock).
 *
 *          Outgoing writes (write()) broadcast to all currently connected clients.
 *          Clients that fail during a write are removed at the end of that write call.
 *
 * @version 2.0
 * @date    04.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>
#include <atomic>
#include <vector>

namespace adam::modules::network
{
    /**
     * @class port_tcp_server
     * @brief TCP Server Port — accepts multiple clients and broadcasts data bidirectionally.
     *
     *        User parameters (configured via the "user_parameters" list):
     *          - local_interface  (string)  — Local IP to listen on (default "0.0.0.0").
     *          - local_port       (integer) — Local port to bind and listen on (default 0).
     *          - tcp_nodelay      (boolean) — Apply TCP_NODELAY to each accepted client socket (default true).
     *          - ip_version       (string)  — "auto", "ipv4", or "ipv6".
     */
    class ADAM_NETWORK_API port_tcp_server : public port_in_out
    {
    public:

        /** @brief Returns the canonical type-name string for this port type. */
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "tcp-server"_ct; }

        /** @brief Returns the static default user-parameter list for this port type. */
        static const configuration_parameter_list& get_user_parameters();

        /**
         * @brief Constructs the port, creates and binds the listener socket.
         * @param item_name  Unique item name for this port instance.
         */
        explicit port_tcp_server(const string_hashed& item_name);

        virtual ~port_tcp_server();

        /** @brief Returns the type-name for this port instance. */
        virtual const string_hashed_ct& get_type_name() const override
        {
            static string_hashed_ct name = type_name();
            return name;
        }

        /**
         * @brief Creates and binds the listener socket, then starts the worker thread.
         * @return True on success, false if socket creation, bind, or listen failed.
         */
        virtual bool start() override;

        /**
         * @brief Closes the listener and all connected client sockets, then stops the thread.
         * @return True if the underlying port::stop() succeeded.
         */
        virtual bool stop() override;

        /**
         * @brief Accepts new clients and reads data from all currently connected clients.
         *
         *        Uses a single select() call across the listener and all client sockets.
         *        Returns after delivering one buffer's worth of data from one client.
         *
         * @param buff  Output: newly allocated buffer containing received data on success.
         *              The caller must release it.
         * @return True if data was received and @p buff is valid.
         */
        virtual bool read(buffer*& buff) override;

        /**
         * @brief Broadcasts @p buff to all currently connected clients.
         *
         *        Clients that fail the send are removed from the client list after the
         *        broadcast completes.  Returns true as long as at least no fatal error
         *        occurred (individual client send failures are handled silently).
         *
         * @param buff  Buffer containing data to send. Must be non-null and non-empty.
         * @return True if the broadcast completed (even if some clients failed).
         */
        virtual bool write(buffer* buff) override;

    private:

        /** @brief The listener socket accepting new connections. Stored as uintptr_t so
         *         it is compatible with atomic operations. Only accessed from start()/stop()
         *         and the worker thread — no spinlock required for m_listener itself. */
        uintptr_t                     m_listener;

        /** @brief List of all currently connected client socket handles. */
        std::vector<uintptr_t>        m_clients;

        /** @brief Spinlock protecting m_clients between the worker thread (read/accept)
         *         and any caller thread invoking write() or stop().
         *         Held for nanoseconds only — never while doing I/O. */
        std::atomic_flag              m_clients_mutex = ATOMIC_FLAG_INIT;

        // --- User-parameter pointers (set in constructor, read-only thereafter) ---
        configuration_parameter_string*  m_local_interface = nullptr; ///< Local interface IP.
        configuration_parameter_integer* m_local_port      = nullptr; ///< Local listen port.
        configuration_parameter_boolean* m_tcp_nodelay     = nullptr; ///< TCP_NODELAY on accepted sockets.
        configuration_parameter_string*  m_ip_version      = nullptr; ///< "auto", "ipv4", "ipv6".
    };

} // namespace adam::modules::network
