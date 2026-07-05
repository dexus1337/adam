#pragma once

/**
 * @file    port-udp-broadcast.hpp
 * @author  dexus1337
 * @brief   Declares the UDP Broadcast Port for IPv4 subnet-broadcast communication.
 *
 *          Inherits the common select/recvfrom read loop and stop() teardown from
 *          port_udp_base.  This port is IPv4-only; UDP broadcast is not supported
 *          on IPv6.  Concrete responsibilities:
 *            - start()  - creates a UDP socket with SO_BROADCAST enabled, then binds it.
 *            - write()  - sends a datagram to the configured broadcast address and port.
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
     * @class port_udp_broadcast
     * @brief UDP Broadcast Port (IPv4 only) - sends and receives UDP subnet-broadcast datagrams.
     *
     *        User parameters (configured via the "user_parameters" list):
     *          - interface        (string)  - Local interface to bind to (default "auto").
     *          - interface_port   (integer) - Local port to bind to (0 = any).
     *          - broadcast_ip     (string)  - Destination broadcast address (default "255.255.255.255").
     *          - remote_port      (integer) - Destination port for outgoing broadcasts.
     *
     *        Note: ip_version is fixed to IPv4; there is no ip_version parameter.
     */
    class ADAM_NETWORK_API port_udp_broadcast : public port_udp_base
    {
    public:

        /** @brief Returns the canonical type-name string for this port type. */
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "udp-broadcast"_ct; }

        /** @brief Returns the static default user-parameter list for this port type. */
        static const configuration_parameter_list& get_user_parameters();

        /**
         * @brief Constructs the port and wires all user-parameter pointers.
         * @param item_name  Unique item name for this port instance.
         */
        explicit port_udp_broadcast(const string_hashed& item_name);

        virtual ~port_udp_broadcast();

        /** @copydoc port_udp_base::get_type_name */
        virtual const string_hashed_ct& get_type_name() const override
        {
            static string_hashed_ct name = type_name();
            return name;
        }

        /**
         * @brief Creates the UDP socket with SO_BROADCAST enabled and binds it to the
         *        configured local IPv4 address and port.
         * @return True on success, false if socket creation, option-setting, or bind failed.
         */
        virtual bool start() override;

        /**
         * @brief Sends @p buff as a broadcast datagram to broadcast_ip / remote_port.
         * @param buff  Buffer containing the data to send. Must be non-null and non-empty.
         * @return True if all bytes were sent successfully.
         */
        virtual bool write(buffer* buff) override;

    private:

        /** @brief IPv4 broadcast destination address (e.g. "255.255.255.255"). */
        configuration_parameter_string*  m_broadcast_ip = nullptr;

        /** @brief Destination port for outgoing broadcast datagrams. */
        configuration_parameter_integer* m_remote_port  = nullptr;
    };

} // namespace adam::modules::network
