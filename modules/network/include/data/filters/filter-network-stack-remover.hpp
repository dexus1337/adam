#pragma once

/**
 * @file    filter-network-stack-remover.hpp
 * @author  dexus1337
 * @brief   Defines a filter that removes both IP and Protocol headers.
 * @version 1.0
 * @date    10.07.2026
 */

#include "api/api-sdk.hpp"
#include "data/processors/filter.hpp"

namespace adam::network
{
    /**
     * @class filter_network_stack_remover
     * @brief A filter that removes both the IP header and transport header (TCP/UDP).
     */
    class filter_network_stack_remover : public filter   
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "filter_network_stack_remover"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        filter_network_stack_remover(const string_hashed& item_name);

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };
        
        virtual bool handle_data(buffer*& buf) override;
    };
}
