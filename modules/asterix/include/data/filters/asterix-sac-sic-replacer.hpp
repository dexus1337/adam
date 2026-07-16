#pragma once

/**
 * @file    asterix-sac-sic-replacer.hpp
 * @author  dexus1337
 * @brief   Defines a filter that replaces SAC/SIC codes in Asterix messages
 * @version 1.0
 * @date    16.07.2026
 */

#include "api/api-asterix.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    /**
     * @class sac_sic_replacer
     * @brief A processor that replaces the SAC and SIC values in an Asterix data stream.
     */
    class ADAM_ASTERIX_API sac_sic_replacer : public adam::filter
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "asterix-sac-sic-replacer"_ct; }
        
        static const configuration_parameter_list& get_user_parameters();

        sac_sic_replacer(const string_hashed& name);
        ~sac_sic_replacer() override = default;

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool handle_data(buffer*& buf) override;

    private:
        adam::configuration_parameter_integer* m_sac_param; /**< Fast-access pointer to the new SAC parameter. */
        adam::configuration_parameter_integer* m_sic_param; /**< Fast-access pointer to the new SIC parameter. */
    };
}
