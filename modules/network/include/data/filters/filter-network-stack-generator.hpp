#pragma once

/**
 * @file    filter-network-stack-generator.hpp
 * @author  dexus1337
 * @brief   Defines a unified filter that generates a network stack (IP + TCP/UDP).
 * @version 1.0
 * @date    10.07.2026
 */

#include "api/api-sdk.hpp"
#include "data/processors/filter.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"

namespace adam::network
{
    /**
     * @class filter_network_stack_generator
     * @brief A unified filter that generates IP and transport (TCP/UDP) headers.
     */
    class filter_network_stack_generator : public filter   
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "filter_network_stack_generator"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        filter_network_stack_generator(const string_hashed& item_name);

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };
        
        virtual bool handle_data(buffer*& buf) override;

    protected:
        configuration_parameter_string* m_ip_version;
        configuration_parameter_string* m_protocol;
        configuration_parameter_string* m_src_ip;
        configuration_parameter_string* m_dst_ip;
        configuration_parameter_integer* m_src_port;
        configuration_parameter_integer* m_dst_port;
        configuration_parameter_integer* m_ttl;
        configuration_parameter_integer* m_tos;
        configuration_parameter_integer* m_tcp_seq;
        configuration_parameter_integer* m_tcp_ack;
        configuration_parameter_integer* m_tcp_flags;
    };
}
