#pragma once

/**
 * @file    asterix-category-filter.hpp
 * @author  dexus1337
 * @brief   Defines a filter that includes or excludes Asterix messages based on their Category.
 * @version 1.0
 * @date    26.08.2026
 */

#include "api/api-asterix.hpp"
#include <adam-core.hpp>

#include <unordered_set>
#include <string>
#include <vector>
#include <string>

namespace adam::modules::asterix
{
    /**
     * @class category_filter
     * @brief A processor that filters ASTERIX records by their CAT Number.
     */
    class ADAM_ASTERIX_API category_filter : public adam::filter
    {
    public:
        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "asterix-category-filter"_ct; }
        
        static const configuration_parameter_list& get_user_parameters();

        category_filter(const string_hashed& name);
        ~category_filter() override = default;

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }
        
        virtual bool handle_data(buffer*& buf) override;

    private:
        void update_parsed_cats();

        adam::configuration_parameter_string* m_mode_param; /**< Fast-access pointer to the mode parameter (whitelist/blacklist). */
        adam::configuration_parameter_string* m_cats_param;  /**< Fast-access pointer to the cats string. */

        string_hashed                 m_last_cats_hash;
        std::unordered_set<uint32_t>  m_parsed_cats;
    };
}
