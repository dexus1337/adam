#pragma once

/**
 * @file    asterix-sac-sic-filter.hpp
 * @author  dexus1337
 * @brief   Defines a filter that includes or excludes Asterix messages based on SAC/SIC patterns.
 * @version 1.0
 * @date    26.08.2026
 */

#include "api/api-asterix.hpp"
#include <adam-core.hpp>

#include <vector>
#include <string>
#include <bitset>

namespace adam::modules::asterix
{
    /**
     * @struct sac_sic_pattern
     * @brief  Represents a parsed SAC/SIC match pattern with wildcard support.
     */
    struct sac_sic_pattern
    {
        bool    any_sac = false;
        bool    any_sic = false;
        uint8_t sac     = 0;
        uint8_t sic     = 0;

        inline bool matches(uint8_t in_sac, uint8_t in_sic) const
        {
            if (!any_sac && sac != in_sac) return false;
            if (!any_sic && sic != in_sic) return false;
            return true;
        }
    };

    /**
     * @class sac_sic_filter
     * @brief A processor that filters ASTERIX records by SAC/SIC patterns using an O(1) bitset lookup table.
     */
    class ADAM_ASTERIX_API sac_sic_filter : public adam::filter
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "asterix-sac-sic-filter"_ct; }
        
        static const configuration_parameter_list& get_user_parameters();

        sac_sic_filter(const string_hashed& name);
        ~sac_sic_filter() override = default;

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool handle_data(buffer*& buf) override;

    private:
        void update_parsed_patterns();

        adam::configuration_parameter_string* m_filter_mode_param; /**< Fast-access pointer to the mode parameter (whitelist/blacklist). */
        adam::configuration_parameter_string* m_sac_sic_param;     /**< Fast-access pointer to the sac/sic pattern string. */

        string_hashed         m_last_sac_sic_hash;
        std::bitset<0xffff>    m_match_lut;                         /**< O(1) 8KB Bitset LUT for all 65,536 (SAC, SIC) pairs. */
        bool                  m_has_patterns = false;
    };
}
