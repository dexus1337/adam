#pragma once

/**
 * @file    asterix-sac-sic-replacer.hpp
 * @author  dexus1337
 * @brief   Defines a filter that replaces SAC/SIC codes in Asterix messages using pattern matching.
 * @version 2.0
 * @date    26.08.2026
 */

#include "api/api-asterix.hpp"
#include <adam-core.hpp>

#include <vector>
#include <string>
#include <bitset>
#include <array>

namespace adam::modules::asterix
{
    /**
     * @struct replacement_entry
     * @brief Precomputed SAC/SIC replacement mapping.
     */
    struct replacement_entry
    {
        uint8_t new_sac = 0;
        uint8_t new_sic = 0;
        bool    changed = false;
    };

    /**
     * @class sac_sic_replacer
     * @brief A processor that replaces SAC and SIC values based on source and target patterns.
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
        void update_parsed_patterns();

        adam::configuration_parameter_string* m_source_param; /**< Source SAC/SIC pattern (e.g. 103/x, x/63, x/x). */
        adam::configuration_parameter_string* m_target_param; /**< Target SAC/SIC to inject (e.g. 105/x, x/200, x/x). */

        string_hashed                                    m_last_source_hash;
        string_hashed                                    m_last_target_hash;
        std::array<replacement_entry, 0xffff>             m_replacement_lut;
        bool                                             m_has_source_patterns = false;
    };
}
