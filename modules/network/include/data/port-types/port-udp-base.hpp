#pragma once

/**
 * @file    port-udp-base.hpp
 * @author  dexus1337
 * @brief   Internal base class shared by all UDP port types (unicast, broadcast, multicast).
 *
 *          Provides a single implementation of the common select/recvfrom read loop,
 *          the stop() teardown, the ip_version configuration preset block, and the
 *          m_socket / m_interface / m_interface_port / m_ip_version members that every
 *          UDP port requires.  Concrete subclasses only need to implement start() and write().
 *
 * @version 1.0
 * @date    04.07.2026
 */

#include "data/port-types/port-network.hpp"

namespace adam::modules::network
{
    /**
     * @class port_udp_base
     * @brief Internal CRTP-free base class for UDP port implementations.
     *
     *        Subclasses must call port_udp_base::init_base_params() from their constructor
     *        to wire the shared parameter pointers after adding their own user_parameters list.
     *
     *        Subclasses must still implement:
     *          - static get_user_parameters()   (to define their specific parameters)
     *          - start()                         (socket creation + protocol-specific options)
     *          - write()                         (sendto with their specific destination)
     */
    class ADAM_NETWORK_API port_udp_base : public port_network
    {
    public:

        /**
         * @brief Constructs the base with an invalid socket handle.
         * @param item_name  The port item name passed to port_in_out.
         */
        explicit port_udp_base(const string_hashed& item_name)
            : port_network(item_name)
            , m_socket(static_cast<uintptr_t>(invalid_socket_val))
        {
        }

        virtual ~port_udp_base() = default;

        // -----------------------------------------------------------------
        // Shared stop() - closes the UDP socket and logs the event.
        // -----------------------------------------------------------------

        /**
         * @brief Closes the UDP socket (if open) and calls port::stop().
         *
         *        Subclasses should NOT override this unless they have additional
         *        teardown logic (e.g. leaving a multicast group). In that case,
         *        call port_udp_base::stop() at the end of the override.
         *
         * @return True if the base port::stop() succeeded.
         */
        virtual bool stop() override;

        // -----------------------------------------------------------------
        // Shared read() - select + recvfrom loop, identical for all UDP types.
        // -----------------------------------------------------------------

        /**
         * @brief Waits for incoming UDP data and returns it in a newly allocated buffer.
         *
         *        The function blocks in a 100 ms select() loop until data arrives or the
         *        port is stopped.  On each call it returns exactly one datagram.
         *
         * @param buff  Output: pointer set to a buffer_manager-allocated buffer on success.
         *              The caller is responsible for releasing it.
         * @return True if a datagram was received and @p buff is valid, false otherwise.
         */
        virtual bool read(buffer*& buff) override;

    protected:

        uintptr_t m_socket; ///< Native socket handle stored as uintptr_t for atomic compatibility.

        // User-parameter pointers - populated by init_base_params().
        configuration_parameter_string*  m_interface = nullptr; ///< Local interface IP to bind to.
        configuration_parameter_integer* m_interface_port      = nullptr; ///< Local port to bind to.
        configuration_parameter_string*  m_ip_version      = nullptr; ///< "ipv4", "ipv6", or "auto".

        // -----------------------------------------------------------------
        // Helpers for subclass constructors
        // -----------------------------------------------------------------

        /**
         * @brief Populates m_interface, m_interface_port, and m_ip_version from
         *        the already-added "user_parameters" configuration list.
         *
         *        Must be called from the subclass constructor after add_parameters().
         *        Subclasses that do not have an ip_version parameter (e.g. broadcast)
         *        may pass has_ip_version = false to skip that pointer.
         *
         * @param has_ip_version  If false, m_ip_version is left as nullptr.
         */
        void init_base_params(bool has_ip_version = true);

        /**
         * @brief Adds the standard "ip_version" preset parameter to the given list.
         *
         *        Centralises the duplicated preset block that previously appeared in
         *        every get_user_parameters() implementation.
         *
         * @param up  The user_parameters list to append to.
         */
        static void add_ip_version_param(adam::configuration_parameter_list_sorted* up);

    };

} // namespace adam::modules::network
