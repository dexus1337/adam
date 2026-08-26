#include "data/filters/asterix-category-filter.hpp"

#include "data/format-asterix.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module-asterix.hpp"
#include "data/asterix-internal.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

#include <memory>
#include <string>
#include <cstring>

namespace adam::modules::asterix
{
    const configuration_parameter_list& category_filter::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container mode_presets = {};
            mode_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("whitelist"_ct, "whitelist"_ct));
            mode_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("blacklist"_ct, "blacklist"_ct));

            auto mode_param = std::make_unique<configuration_parameter_string>("mode"_ct, "whitelist"_ct, std::move(mode_presets));
            mode_param->set_description(language_english, "The filtering mode: whitelist (only allow listed Categories) or blacklist (drop listed Categories)."_ct);
            mode_param->set_description(language_german, "Der Filtermodus: Whitelist (nur gelistete Kategorien zulassen) oder Blacklist (gelistete Kategorien verwerfen)."_ct);
            up->add(std::move(mode_param));

            auto cats_param = std::make_unique<configuration_parameter_string>("cats"_ct, ""_ct);
            cats_param->set_description(language_english, "Semicolon or comma separated list of ASTERIX Categories (e.g. 34; 48)."_ct);
            cats_param->set_description(language_german, "Durch Semikolon oder Komma getrennte Liste von ASTERIX Kategorien (z.B. 34; 48)."_ct);
            up->add(std::move(cats_param));
            
            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    category_filter::category_filter(const string_hashed& name) : filter(name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        m_format_input  = &data_format_asterix;
        m_format_output = &data_format_asterix;

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        m_mode_param = user_params->get<configuration_parameter_string>("mode"_ct);
        m_cats_param  = user_params->get<configuration_parameter_string>("cats"_ct);
    }

    void category_filter::update_parsed_cats()
    {
        if (m_last_cats_hash == m_cats_param->get_value()) return;

        m_last_cats_hash = m_cats_param->get_value();
        std::string current_cats_str = std::string(m_last_cats_hash);
        m_parsed_cats.clear();

        size_t start = 0;
        size_t end = current_cats_str.find_first_of(";,");
        while (end != std::string::npos)
        {
            std::string id_str = current_cats_str.substr(start, end - start);
            try { if (!id_str.empty()) m_parsed_cats.insert(std::stoul(id_str, nullptr, 0)); } catch (...) {}
            start = end + 1;
            end = current_cats_str.find_first_of(";,", start);
        }
        std::string id_str = current_cats_str.substr(start);
        try { if (!id_str.empty()) m_parsed_cats.insert(std::stoul(id_str, nullptr, 0)); } catch (...) {}
    }

    bool category_filter::handle_data(buffer*& buf)
    {
        if (!buf) return false;

        auto* root_frame = buf->begin_as<frame>();
        if (!root_frame) return false;

        update_parsed_cats();

        const bool is_whitelist = (m_mode_param->get_value() == "whitelist"_ct);

        auto* stats = get_state_buffer_data();
        if (stats)
        {
            stats->total_buffers_recieved++;
            stats->total_bytes_recieved += buf->get_size();
        }

        uint32_t total_blocks = 0;
        uint32_t kept_blocks = 0;
        uint32_t removed_blocks = 0;

        for (auto& blk : *root_frame)
        {
            if (blk.is_removed()) continue;

            total_blocks++;
            const bool match = (m_parsed_cats.find(blk.category) != m_parsed_cats.end());
            const bool keep  = is_whitelist ? match : !match;

            if (keep)
            {
                kept_blocks++;
            }
            else
            {
                blk.set_removed(true);
                removed_blocks++;
            }
        }

        if (kept_blocks == 0)
        {
            if (stats)
            {
                stats->total_buffers_discarded++;
                stats->total_bytes_discarded += buf->get_size();
            }
            return false;
        }

        if (removed_blocks > 0)
        {
            root_frame->set_modified(true);
        }

        if (stats)
        {
            stats->total_buffers_forwarded++;
            stats->total_bytes_forwarded += buf->get_size();
        }

        return true;
    }
}
