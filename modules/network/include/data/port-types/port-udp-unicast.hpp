#pragma once

/**
 * @file    port-udp-unicast.hpp
 * @author  dexus1337
 * @brief   Declares the UDP Unicast Port for bidirectional point-to-point UDP communication.
 *
 *          Inherits the common select/recvfrom read loop and stop() teardown from
 *          port_udp_base.  Concrete responsibilities:
 *            - start()  — creates and binds a UDP socket to the configured local interface/port.
 *            - write()  — sends a datagram to the configured remote IP and port.
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
     * @class port_udp_unicast
     * @brief UDP Unicast Port — sends and receives standard point-to-point UDP datagrams.
     *
     *        User parameters (configured via the "user_parameters" list):
     *          - interface        (string)  — Local interface to bind to (default "auto").
     *          - interface_port   (integer) — Local port to bind to (0 = any).
     *          - remote_ip        (string)  — Destination IP for outgoing datagrams.
     *          - remote_port      (integer) — Destination port for outgoing datagrams.
     *          - ip_version       (string)  — "auto", "ipv4", or "ipv6".
     */
    class ADAM_NETWORK_API port_udp_unicast : public port_udp_base
    {
    public:

        /** @brief Returns the canonical type-name string for this port type. */
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "udp-unicast"_ct; }

        /** @brief Returns the static default user-parameter list for this port type. */
        static const configuration_parameter_list& get_user_parameters();

        /**
         * @brief Constructs the port and wires all user-parameter pointers.
         * @param item_name  Unique item name for this port instance.
         */
        explicit port_udp_unicast(const string_hashed& item_name);

        virtual ~port_udp_unicast();

        /** @copydoc port_udp_base::get_type_name */
        virtual const string_hashed_ct& get_type_name() const override
        {
            static string_hashed_ct name = type_name();
            return name;
        }

        /**
         * @brief Creates and binds the UDP socket to the configured local address/port.
         * @return True on success, false if socket creation or bind failed.
         */
        virtual bool start() override;

        /**
         * @brief Sends @p buff as a UDP datagram to the configured remote_ip / remote_port.
         * @param buff  Buffer containing the data to send. Must be non-null and non-empty.
         * @return True if all bytes were sent successfully.
         */
        virtual bool write(buffer* buff) override;

    private:

        /** @brief Destination IP address for outgoing datagrams. */
        configuration_parameter_string*  m_remote_ip   = nullptr;

        /** @brief Destination port for outgoing datagrams. */
        configuration_parameter_integer* m_remote_port = nullptr;
    };

} // namespace adam::modules::network
