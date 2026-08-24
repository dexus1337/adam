#pragma once

/**
 * @file    can-parser.hpp
 * @author  dexus1337
 * @brief   Defines the parser for the CAN data format with profile selection support.
 * @version 1.0
 * @date    24.08.2026
 */

#include "api/api-can.hpp"
#include <adam-core.hpp>

namespace adam::modules::can
{
    class can_profile;

    /**
     * @class can_parser
     * @brief Parses and validates raw CAN message buffers with selectable CAN profile definitions.
     */
    class ADAM_CAN_API can_parser : public adam::parser
    {
    public:
        static const adam::configuration_parameter_list& get_user_parameters();
        static const adam::configuration_parameter_list& get_default_parameters();

        can_parser(const adam::string_hashed& item_name = "can_parser"_ct, const adam::configuration_parameter_list& default_params = get_default_parameters());
        ~can_parser() override = default;

        /**
         * @brief Parses and validates the provided buffer containing raw CAN data.
         * @param buf The buffer containing raw CAN frames.
         * @param internal_data The output buffer allocated by the parser.
         * @return True if parsing succeeded, false otherwise.
         */
        bool parse(class adam::buffer* buf, class adam::buffer*& internal_data) override;

        /**
         * @brief Gets the currently selected CAN profile based on parser configuration.
         * @return Pointer to active CAN profile, or nullptr if none selected.
         */
        const can_profile* get_selected_profile() const;

    private:
        adam::configuration_parameter_string* m_profile_param;
    };
}
