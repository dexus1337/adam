#pragma once

/**
 * @file    can-message-filter.hpp
 * @author  Antigravity
 * @brief   Defines a filter that includes or excludes CAN messages based on their ID.
 * @version 1.0
 * @date    19.07.2026
 */

#include "api/api-can.hpp"
#include "data/can-types.hpp"
#include <adam-sdk.hpp>

#include <unordered_set>
#include <string>
#include <vector>
#include <string>

namespace adam::modules::can
{
    /**
     * @class can_message_filter
     * @brief A processor that filters CAN messages by their ID.
     */
    class ADAM_CAN_API can_message_filter : public adam::filter
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "can-message-filter"_ct; }
        
        static const configuration_parameter_list& get_user_parameters();

        can_message_filter(const string_hashed& name);
        ~can_message_filter() override = default;

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool handle_data(buffer*& buf) override;

    private:
        void update_parsed_ids();

        adam::configuration_parameter_string* m_mode_param; /**< Fast-access pointer to the mode parameter (whitelist/blacklist). */
        adam::configuration_parameter_string* m_ids_param;  /**< Fast-access pointer to the IDs string. */

        string_hashed                 m_last_ids_hash;
        std::unordered_set<uint32_t>  m_parsed_ids;
    };
}
