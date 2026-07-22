#include "data/filters/can-message-filter.hpp"

#include "data/format-can.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module-can.hpp"
#include "data/can-types.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

#include <memory>
#include <string>

namespace adam::modules::can
{
    const configuration_parameter_list& can_message_filter::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container mode_presets = {};
            mode_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("whitelist"_ct, "whitelist"_ct));
            mode_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("blacklist"_ct, "blacklist"_ct));

            auto mode_param = std::make_unique<configuration_parameter_string>("mode"_ct, "whitelist"_ct, std::move(mode_presets));
            mode_param->set_description(language_english, "The filtering mode: whitelist (only allow listed IDs) or blacklist (drop listed IDs)."_ct);
            mode_param->set_description(language_german, "Der Filtermodus: Whitelist (nur gelistete IDs zulassen) oder Blacklist (gelistete IDs verwerfen)."_ct);
            up->add(std::move(mode_param));

            auto ids_param = std::make_unique<configuration_parameter_string>("ids"_ct, ""_ct);
            ids_param->set_description(language_english, "Semicolon or comma separated list of CAN message IDs (e.g. 0x1A; 0x1B; 100)."_ct);
            ids_param->set_description(language_german, "Durch Semikolon oder Komma getrennte Liste von CAN-Nachrichten-IDs (z.B. 0x1A; 0x1B; 100)."_ct);
            up->add(std::move(ids_param));
            
            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    can_message_filter::can_message_filter(const string_hashed& name) : filter(name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        m_format_input  = &data_format_can;
        m_format_output = &data_format_can;

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        m_mode_param = user_params->get<configuration_parameter_string>("mode"_ct);
        m_ids_param  = user_params->get<configuration_parameter_string>("ids"_ct);
    }

    void can_message_filter::update_parsed_ids()
    {
        if (m_last_ids_hash == m_ids_param->get_value())
            return;

        m_last_ids_hash = m_ids_param->get_value();
        std::string current_ids_str = std::string(m_last_ids_hash);
        m_parsed_ids.clear();

        size_t start = 0;
        size_t end = current_ids_str.find_first_of(";,");
        while (end != std::string::npos)
        {
            std::string id_str = current_ids_str.substr(start, end - start);
            try { if (!id_str.empty()) m_parsed_ids.insert(std::stoul(id_str, nullptr, 0)); } catch (...) {}
            start = end + 1;
            end = current_ids_str.find_first_of(";,", start);
        }
        std::string id_str = current_ids_str.substr(start);
        try { if (!id_str.empty()) m_parsed_ids.insert(std::stoul(id_str, nullptr, 0)); } catch (...) {}
    }

    bool can_message_filter::handle_data(buffer*& buf)
    {
        if (!buf) return false;

        update_parsed_ids();

        const bool is_whitelist = (m_mode_param->get_value() == "whitelist"_ct);

        auto* orig_messages = buf->get_begin_as<const can_message>();
        uint32_t message_count = buf->get_size() / sizeof(can_message);

        if (message_count == 0) return false;

        uint32_t first_kept = message_count;
        uint32_t last_kept = 0;
        uint32_t kept_count = 0;

        for (uint32_t i = 0; i < message_count; ++i)
        {
            uint32_t id = orig_messages[i].get_id();
            bool contains = (m_parsed_ids.find(id) != m_parsed_ids.end());

            if ((is_whitelist && contains) || (!is_whitelist && !contains))
            {
                if (first_kept == message_count)
                    first_kept = i;
                last_kept = i;
                kept_count++;
            }
        }

        if (kept_count == 0)
        {
            buf->release();
            buf = nullptr;
            return false; 
        }

        if (kept_count == message_count)
            return true;

        if (kept_count == (last_kept - first_kept + 1))
        {
            buf->move_start_pos(first_kept * sizeof(can_message));
            buf->set_size(kept_count * sizeof(can_message));
            return true;
        }

        auto new_size   = static_cast<uint32_t>(kept_count * sizeof(can_message));
        buffer* new_buf = buffer_manager::get().request_buffer(new_size);
        if (!new_buf)
            return true;

        new_buf->set_timestamp(buf->get_timestamp());
        new_buf->set_data_format(buf->get_data_format());
        new_buf->set_start_pos(0);
        new_buf->set_size(new_size);

        auto* new_messages = new_buf->begin_as<can_message>();
        uint32_t write_idx = 0;
        for (uint32_t i = first_kept; i <= last_kept; ++i)
        {
            uint32_t id = orig_messages[i].get_id();
            bool contains = (m_parsed_ids.find(id) != m_parsed_ids.end());

            if ((is_whitelist && contains) || (!is_whitelist && !contains))
                new_messages[write_idx++] = orig_messages[i];
        }

        buf->release();
        buf = new_buf;

        return true;
    }
}
