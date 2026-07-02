#pragma once

/**
 * @file    port-udp-unicast.hpp
 * @author  dexus1337
 * @brief   Defines a UDP Unicast Port for bidirectional IP communication.
 * @version 1.0
 * @date    02.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::network
{
    /**
     * @class port_udp_unicast
     * @brief Defines a UDP Unicast Port for bidirectional IP communication.
     */
    class ADAM_NETWORK_API port_udp_unicast : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "udp-unicast"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        port_udp_unicast(const string_hashed& item_name);
        virtual ~port_udp_unicast();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool start() override;
        virtual bool stop() override;
        virtual bool read(buffer*& buff) override;
        virtual bool write(buffer* buff) override;

    private:

        uintptr_t m_socket;

        configuration_parameter_string* m_local_interface = nullptr;
        configuration_parameter_integer* m_local_port = nullptr;
        configuration_parameter_string* m_remote_ip = nullptr;
        configuration_parameter_integer* m_remote_port = nullptr;
        configuration_parameter_string* m_ip_version = nullptr;
    };
}
