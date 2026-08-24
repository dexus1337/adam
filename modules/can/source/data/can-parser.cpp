#include "data/can-parser.hpp"
#include "data/can-types.hpp"
#include "data/can-profile.hpp"
#include "data/format-can.hpp"
#include <adam-core.hpp>

/**
 * @file    can-parser.cpp
 * @author  dexus1337
 * @brief   Implements the CAN parser and profile parameter selection.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can
{
    using namespace adam::string_hashed_ct_literals;

    const adam::configuration_parameter_list& can_parser::get_user_parameters()
    {
        static const adam::configuration_parameter_list params = []()
        {
            adam::configuration_parameter_list p;
            auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);

            const auto* default_prof = can_profile_pool::get().get_default_profile();
            const string_hashed_ct default_name = default_prof ? default_prof->get_name() : "None"_ct;

            auto profile_param = std::make_unique<adam::configuration_parameter_string>("profile"_ct, default_name);
            profile_param->set_mode(adam::configuration_parameter_string::value_mode_preset);
            profile_param->set_description(adam::language_english, "Selected CAN profile for signal decoding"_ct);
            profile_param->set_description(adam::language_german, "Ausgewähltes CAN-Profil für Signaldekodierung"_ct);

            for (const auto* prof : can_profile_pool::get().get_profiles())
            {
                profile_param->add_preset(std::make_unique<adam::configuration_parameter_string>(prof->get_name(), prof->get_name()));
            }

            user_params->add(std::move(profile_param));
            p.add(std::move(user_params));
            return p;
        }();
        return params;
    }

    const adam::configuration_parameter_list& can_parser::get_default_parameters()
    {
        return get_user_parameters();
    }

    can_parser::can_parser(const adam::string_hashed& item_name, const adam::configuration_parameter_list& default_params)
        : adam::parser(item_name, default_params),
          m_profile_param(nullptr)
    {
        auto* user_params = get_parameter<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
        if (user_params)
        {
            m_profile_param = dynamic_cast<adam::configuration_parameter_string*>(user_params->get("profile"_ct));
        }
    }

    const can_profile* can_parser::get_selected_profile() const
    {
        if (!m_profile_param)
        {
            return can_profile_pool::get().get_default_profile();
        }

        const auto* prof = can_profile_pool::get().get_profile(m_profile_param->get_value().get_hash());
        if (prof)
        {
            return prof;
        }

        return can_profile_pool::get().get_default_profile();
    }

    bool can_parser::parse(class adam::buffer* buf, class adam::buffer*& internal_data)
    {
        if (!buf)
        {
            return false;
        }

        const uint8_t* current = buf->get_begin_as<uint8_t>();
        const uint8_t* end = current + buf->get_size();

        // Validate CAN messages in buffer
        while (current + sizeof(can_message) <= end)
        {
            const auto* msg = reinterpret_cast<const can_message*>(current);
            uint8_t len = msg->get_length();
            if (current + len > end)
            {
                return false;
            }

            current += len;
        }

        if (!internal_data || internal_data->get_capacity() < buf->get_size())
        {
            if (internal_data)
            {
                internal_data->release();
            }

            internal_data = adam::buffer_manager::get().request_buffer(static_cast<uint32_t>(buf->get_size()));
            if (!internal_data)
            {
                return false;
            }
        }

        std::memcpy(internal_data->data(), buf->get_data(), buf->get_size());
        internal_data->set_size(buf->get_size());
        internal_data->set_referenced_buffer(buf);
        internal_data->set_timestamp(buf->get_timestamp());
        internal_data->set_data_format(&data_format_can);

        return true;
    }
}
