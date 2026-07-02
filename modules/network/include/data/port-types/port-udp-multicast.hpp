#pragma once

/**
 * @file    port-udp-multicast.hpp
 * @author  dexus1337
 * @brief   Defines a UDP Multicast Port for bidirectional IP communication.
 * @version 1.0
 * @date    02.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::network
{
    /**
     * @class port_udp_multicast
     * @brief Defines a UDP Multicast Port for bidirectional IP communication.
     */
    class ADAM_NETWORK_API port_udp_multicast : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "udp-multicast"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        port_udp_multicast(const string_hashed& item_name);
        virtual ~port_udp_multicast();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool start() override;
        virtual bool stop() override;
        virtual bool read(buffer*& buff) override;
        virtual bool write(buffer* buff) override;

    private:

        uintptr_t m_socket;

        configuration_parameter_string* m_local_interface = nullptr;
        configuration_parameter_integer* m_local_port = nullptr;
        configuration_parameter_string* m_multicast_ip = nullptr;
        configuration_parameter_integer* m_multicast_port = nullptr;
        configuration_parameter_integer* m_ttl = nullptr;
        configuration_parameter_boolean* m_loopback = nullptr;
        configuration_parameter_string* m_ip_version = nullptr;
    };
}
