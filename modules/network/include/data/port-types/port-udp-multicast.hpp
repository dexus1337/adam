#pragma once

/**
 * @file    port-udp-multicast.hpp
 * @author  dexus1337
 * @brief   Declares the UDP Multicast Port for IPv4 and IPv6 multicast group communication.
 *
 *          Inherits the common select/recvfrom read loop and stop() teardown from
 *          port_udp_base.  Concrete responsibilities:
 *            - start()  - creates a UDP socket, joins the multicast group, sets TTL/hops
 *                         and loopback options, then binds.
 *            - write()  - sends a datagram to the multicast group address and port.
 *
 * @version 2.0
 * @date    04.07.2026
 */

#include "api/api-network.hpp"
#include "data/port-types/port-udp-base.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::network
{
    /**
     * @class port_udp_multicast
     * @brief UDP Multicast Port - sends and receives UDP multicast datagrams on IPv4 or IPv6.
     *
     *        User parameters (configured via the "user_parameters" list):
     *          - interface        (string)  - Local interface to bind to (default "auto").
     *          - interface_port   (integer) - Local port to bind to (usually the same as multicast_port).
     *          - multicast_ip     (string)  - Multicast group address (e.g. "239.0.0.1" or "ff02::1").
     *          - multicast_port   (integer) - Destination multicast port for outgoing datagrams.
     *          - ttl              (integer) - Multicast Time-To-Live / hop limit (default 1).
     *          - loopback         (boolean) - Whether sent datagrams loop back to local sockets (default true).
     *          - ip_version       (string)  - "auto", "ipv4", or "ipv6".
     */
    class ADAM_NETWORK_API port_udp_multicast : public port_udp_base
    {
    public:

        /** @brief Returns the canonical type-name string for this port type. */
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "udp-multicast"_ct; }

        /** @brief Returns the static default user-parameter list for this port type. */
        static const configuration_parameter_list& get_user_parameters();

        /**
         * @brief Constructs the port and wires all user-parameter pointers.
         * @param item_name  Unique item name for this port instance.
         */
        explicit port_udp_multicast(const string_hashed& item_name);

        virtual ~port_udp_multicast();

        /** @copydoc port_udp_base::get_type_name */
        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };

        /**
         * @brief Creates the UDP socket, joins the configured multicast group, sets
         *        TTL/hop and loopback options, and binds to the local address/port.
         * @return True on success, false on any socket or group-join error.
         */
        virtual bool start() override;

        /**
         * @brief Sends @p buff as a UDP datagram to the multicast group address and port.
         * @param buff  Buffer containing the data to send. Must be non-null and non-empty.
         * @return True if all bytes were sent successfully.
         */
        virtual bool write(buffer* buff) override;

    private:

        /** @brief Multicast group address (IPv4 or IPv6). */
        configuration_parameter_string*  m_multicast_ip   = nullptr;

        /** @brief Destination multicast port for outgoing datagrams. */
        configuration_parameter_integer* m_multicast_port = nullptr;

        /** @brief Multicast TTL / IPv6 hop limit. */
        configuration_parameter_integer* m_ttl            = nullptr;

        /** @brief Whether to loop back sent multicast datagrams to local sockets. */
        configuration_parameter_boolean* m_loopback       = nullptr;
    };

} // namespace adam::modules::network
