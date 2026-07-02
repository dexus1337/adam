#pragma once

/**
 * @file    port-tcp-client.hpp
 * @author  dexus1337
 * @brief   Defines a TCP Client Port for bidirectional IP communication with auto-reconnect.
 * @version 1.0
 * @date    02.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>
#include <mutex>

namespace adam::modules::network
{
    /**
     * @class port_tcp_client
     * @brief Defines a TCP Client Port for bidirectional IP communication with auto-reconnect.
     */
    class ADAM_NETWORK_API port_tcp_client : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "tcp-client"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        port_tcp_client(const string_hashed& item_name);
        virtual ~port_tcp_client();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool start() override;
        virtual bool stop() override;
        virtual bool read(buffer*& buff) override;
        virtual bool write(buffer* buff) override;

    private:

        bool connect();

        uintptr_t m_socket;
        std::mutex m_write_mutex;

        configuration_parameter_string* m_local_interface = nullptr;
        configuration_parameter_string* m_remote_ip = nullptr;
        configuration_parameter_integer* m_remote_port = nullptr;
        configuration_parameter_integer* m_reconnect_interval_ms = nullptr;
        configuration_parameter_boolean* m_tcp_nodelay = nullptr;
        configuration_parameter_string* m_ip_version = nullptr;
    };
}
