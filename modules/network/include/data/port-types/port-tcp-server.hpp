#pragma once

/**
 * @file    port-tcp-server.hpp
 * @author  dexus1337
 * @brief   Defines a TCP Server Port that accepts multiple simultaneous clients and broadcasts writes.
 * @version 1.0
 * @date    02.07.2026
 */

#include "api/api-network.hpp"
#include <adam-sdk.hpp>
#include <mutex>
#include <vector>

namespace adam::modules::network
{
    /**
     * @class port_tcp_server
     * @brief Defines a TCP Server Port that accepts multiple simultaneous clients and broadcasts writes.
     */
    class ADAM_NETWORK_API port_tcp_server : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "tcp-server"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        port_tcp_server(const string_hashed& item_name);
        virtual ~port_tcp_server();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool start() override;
        virtual bool stop() override;
        virtual bool read(buffer*& buff) override;
        virtual bool write(buffer* buff) override;

    private:

        uintptr_t m_listener;
        std::vector<uintptr_t> m_clients;
        std::mutex m_clients_mutex;

        configuration_parameter_string* m_local_interface = nullptr;
        configuration_parameter_integer* m_local_port = nullptr;
        configuration_parameter_boolean* m_tcp_nodelay = nullptr;
        configuration_parameter_string* m_ip_version = nullptr;
    };
}
