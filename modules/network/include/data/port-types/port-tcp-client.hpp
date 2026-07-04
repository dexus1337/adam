#pragma once

/**
 * @file    port-tcp-client.hpp
 * @author  dexus1337
 * @brief   Declares the TCP Client Port for bidirectional TCP communication with auto-reconnect.
 *
 *          The port runs a single background worker thread that:
 *            1. Connects to the configured remote address (with a 2-second connect timeout).
 *            2. Reads incoming data in a non-blocking select() loop.
 *            3. Automatically reconnects after any disconnect, with a configurable interval.
 *
 *          Outgoing writes (write()) are fully independent of the read loop and protected
 *          by a dedicated spinlock covering only the socket-handle read.
 *
 * @version 2.0
 * @date    04.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>
#include <atomic>

namespace adam::modules::network
{
    /**
     * @class port_tcp_client
     * @brief TCP Client Port — connects to a remote TCP server and exchanges data bidirectionally.
     *
     *        User parameters (configured via the "user_parameters" list):
     *          - interface            (string)  — Local interface to bind before connecting (default "auto").
     *          - remote_ip            (string)  — Remote server IP or hostname (default "127.0.0.1").
     *          - remote_port          (integer) — Remote server port (default 0).
     *          - reconnect_interval_ms (integer) — Milliseconds between reconnect attempts (0 = disabled, default 2000).
     *          - tcp_nodelay          (boolean) — Disable Nagle's algorithm for lower latency (default true).
     *          - ip_version           (string)  — "auto", "ipv4", or "ipv6".
     */
    class ADAM_NETWORK_API port_tcp_client : public port_in_out
    {
    public:

        /** @brief Returns the canonical type-name string for this port type. */
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "tcp-client"_ct; }

        /** @brief Returns the static default user-parameter list for this port type. */
        static const configuration_parameter_list& get_user_parameters();

        /**
         * @brief Constructs the port and wires all user-parameter pointers.
         * @param item_name  Unique item name for this port instance.
         */
        explicit port_tcp_client(const string_hashed& item_name);

        virtual ~port_tcp_client();

        /** @brief Returns the type-name for this port instance. */
        virtual const string_hashed_ct& get_type_name() const override
        {
            static string_hashed_ct name = type_name();
            return name;
        }

        /**
         * @brief Starts the background worker thread that manages connect and read.
         * @return True on success.
         */
        virtual bool start() override;

        /**
         * @brief Closes the socket (if open) and stops the background worker thread.
         * @return True if the underlying port::stop() succeeded.
         */
        virtual bool stop() override;

        /**
         * @brief Receives the next available TCP data from the server.
         *
         *        Blocks in a 100 ms select() loop until data arrives, the connection is lost,
         *        or the port is stopped.  On connection loss, the socket is invalidated so the
         *        next call will trigger a reconnect attempt.
         *
         * @param buff  Output: newly allocated buffer containing received data on success.
         *              The caller must release it.
         * @return True if data was received and @p buff is valid.
         */
        virtual bool read(buffer*& buff) override;

        /**
         * @brief Sends @p buff to the connected server.
         *
         *        Uses a spinlock to read the current socket handle safely.
         *        Returns false immediately (without blocking) if not currently connected.
         *
         * @param buff  Buffer containing data to send. Must be non-null and non-empty.
         * @return True if all bytes were sent successfully.
         */
        virtual bool write(buffer* buff) override;

    private:

        /**
         * @brief Resolves the remote address, creates a TCP socket, and connects.
         *        Called by read() whenever m_socket is INVALID_SOCKET_VAL.
         * @return True if the connection was established successfully.
         */
        bool connect();

        // --- Socket handle ---
        /** @brief Native socket handle stored as uintptr_t. Accessed from both the worker
         *         thread (read/connect) and the caller thread (write/stop) under m_write_mutex. */
        uintptr_t                m_socket;

        /** @brief Spinlock protecting reads and writes of m_socket between the worker thread
         *         and any caller invoking write() or stop(). Held for nanoseconds only. */
        std::atomic_flag         m_write_mutex = ATOMIC_FLAG_INIT;

        // --- User-parameter pointers (set in constructor, read-only thereafter) ---
        configuration_parameter_string*  m_interface            = nullptr; ///< Optional local interface IP.
        configuration_parameter_string*  m_remote_ip            = nullptr; ///< Remote server address.
        configuration_parameter_integer* m_remote_port          = nullptr; ///< Remote server port.
        configuration_parameter_integer* m_reconnect_interval_ms = nullptr; ///< Reconnect retry delay in ms.
        configuration_parameter_boolean* m_tcp_nodelay          = nullptr; ///< TCP_NODELAY flag.
        configuration_parameter_string*  m_ip_version           = nullptr; ///< "auto", "ipv4", "ipv6".
    };

} // namespace adam::modules::network
