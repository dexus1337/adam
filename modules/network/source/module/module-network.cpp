#include "module/module-network.hpp"

#include "data/port-types/port-udp-unicast.hpp"
#include "data/port-types/port-udp-multicast.hpp"
#include "data/port-types/port-udp-broadcast.hpp"
#include "data/port-types/port-tcp-client.hpp"
#include "data/port-types/port-tcp-server.hpp"

#include "data/filters/filter-network-stack-remover.hpp"
#include "data/filters/filter-network-stack-generator.hpp"

#if defined(ADAM_PLATFORM_WINDOWS)
#include <winsock2.h>
#endif

namespace adam::modules::network
{
    static module_network global_instance = module_network();

    static default_factory<port, port_udp_unicast>   global_port_udp_unicast_factory;
    static default_factory<port, port_udp_multicast> global_port_udp_multicast_factory;
    static default_factory<port, port_udp_broadcast> global_port_udp_broadcast_factory;
    static default_factory<port, port_tcp_client>    global_port_tcp_client_factory;
    static default_factory<port, port_tcp_server>    global_port_tcp_server_factory;

    static default_factory<processor, adam::network::filter_network_stack_remover> global_filter_network_stack_remover_factory;
    static default_factory<processor, adam::network::filter_network_stack_generator> global_filter_network_stack_generator_factory;

    module_network::module_network() : module("network", version)
    {
        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides support for IP communication over UDP and TCP networks.\n"
                        "Includes unicast, multicast, broadcast, and client/server endpoints.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet Unterstützung für IP-Kommunikation über UDP- und TCP-Netzwerke.\n"
                        "Umfasst Unicast-, Multicast-, Broadcast- und Client/Server-Endpunkte.");

        #if defined(ADAM_PLATFORM_WINDOWS)
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif

        m_port_factories.emplace
        (
            port_udp_unicast::type_name(), 
            registry::factory_data_port(&global_port_udp_unicast_factory, &port_udp_unicast::get_user_parameters(), port_udp_unicast::direction)
        );

        m_port_factories.emplace
        (
            port_udp_multicast::type_name(), 
            registry::factory_data_port(&global_port_udp_multicast_factory, &port_udp_multicast::get_user_parameters(), port_udp_multicast::direction)
        );

        m_port_factories.emplace
        (
            port_udp_broadcast::type_name(), 
            registry::factory_data_port(&global_port_udp_broadcast_factory, &port_udp_broadcast::get_user_parameters(), port_udp_broadcast::direction)
        );

        m_port_factories.emplace
        (
            port_tcp_client::type_name(), 
            registry::factory_data_port(&global_port_tcp_client_factory, &port_tcp_client::get_user_parameters(), port_tcp_client::direction)
        );

        m_port_factories.emplace
        (
            port_tcp_server::type_name(), 
            registry::factory_data_port(&global_port_tcp_server_factory, &port_tcp_server::get_user_parameters(), port_tcp_server::direction)
        );

        m_processor_factories.emplace
        (
            adam::network::filter_network_stack_remover::type_name(), 
            registry::factory_data_processor(&global_filter_network_stack_remover_factory, &adam::network::filter_network_stack_remover::get_user_parameters())
        );

        m_processor_factories.emplace
        (
            adam::network::filter_network_stack_generator::type_name(), 
            registry::factory_data_processor(&global_filter_network_stack_generator_factory, &adam::network::filter_network_stack_generator::get_user_parameters())
        );
    }

    module_network::~module_network() 
    {
        #if defined(ADAM_PLATFORM_WINDOWS)
        WSACleanup();
        #endif
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &adam::modules::network::global_instance;
}