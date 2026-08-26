#include "data/filters/asterix-sac-sic-filter.hpp"

#include "data/format-asterix.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module-asterix.hpp"
#include "data/asterix-internal.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

#include <memory>
#include <string>
#include <algorithm>
#include <cctype>

namespace adam::modules::asterix
{
    const configuration_parameter_list& sac_sic_filter::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container mode_presets = {};
            mode_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("whitelist"_ct, "whitelist"_ct));
            mode_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("blacklist"_ct, "blacklist"_ct));

            auto mode_param = std::make_unique<configuration_parameter_string>("filter_mode"_ct, "whitelist"_ct, std::move(mode_presets));
            mode_param->set_description(language_english, "The filtering mode: whitelist (only allow matching SAC/SIC) or blacklist (drop matching SAC/SIC)."_ct);
            mode_param->set_description(language_german, "Der Filtermodus: Whitelist (nur passende SAC/SIC zulassen) oder Blacklist (passende SAC/SIC verwerfen)."_ct);
            up->add(std::move(mode_param));

            auto sac_sic_param = std::make_unique<configuration_parameter_string>("sac_sic"_ct, ""_ct);
            sac_sic_param->set_description(language_english, "Semicolon or comma separated list of SAC/SIC patterns (e.g. 103/x; x/63; 103/200)."_ct);
            sac_sic_param->set_description(language_german, "Durch Semikolon oder Komma getrennte Liste von SAC/SIC-Mustern (z.B. 103/x; x/63; 103/200)."_ct);
            up->add(std::move(sac_sic_param));
            
            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    sac_sic_filter::sac_sic_filter(const string_hashed& name) : filter(name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        m_format_input  = &data_format_asterix;
        m_format_output = &data_format_asterix;

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        m_filter_mode_param = user_params->get<configuration_parameter_string>("filter_mode"_ct);
        m_sac_sic_param     = user_params->get<configuration_parameter_string>("sac_sic"_ct);
    }

    static std::string trim_string(const std::string& str)
    {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    static bool is_wildcard(const std::string& str)
    {
        if (str.empty()) return true;
        if (str == "x" || str == "X" || str == "*") return true;
        return false;
    }

    static bool parse_uint8(const std::string& str, uint8_t& out_val)
    {
        std::string s = trim_string(str);
        if (s.empty()) return false;

        int base = 10;
        size_t offset = 0;

        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            base = 16;
            offset = 2;
        }
        else if (s.size() >= 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
        {
            base = 2;
            offset = 2;
        }

        try
        {
            unsigned long val = std::stoul(s.substr(offset), nullptr, base);
            if (val > 255) return false;
            out_val = static_cast<uint8_t>(val);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void sac_sic_filter::update_parsed_patterns()
    {
        if (m_last_sac_sic_hash == m_sac_sic_param->get_value()) return;

        m_last_sac_sic_hash = m_sac_sic_param->get_value();
        std::string current_str = std::string(m_last_sac_sic_hash);

        m_match_lut.reset();
        m_has_patterns = false;

        auto parse_token = [this](const std::string& token)
        {
            std::string trimmed = trim_string(token);
            if (trimmed.empty()) return;

            size_t slash_pos = trimmed.find_first_of("/:\\");
            bool any_sac = false;
            bool any_sic = false;
            uint8_t sac_val = 0;
            uint8_t sic_val = 0;

            if (slash_pos == std::string::npos)
            {
                std::string part = trim_string(trimmed);
                if (is_wildcard(part))
                {
                    any_sac = true;
                    any_sic = true;
                }
                else
                {
                    if (!parse_uint8(part, sac_val)) return;
                    any_sic = true;
                }
            }
            else
            {
                std::string sac_str = trim_string(trimmed.substr(0, slash_pos));
                std::string sic_str = trim_string(trimmed.substr(slash_pos + 1));

                if (is_wildcard(sac_str))
                {
                    any_sac = true;
                }
                else
                {
                    if (!parse_uint8(sac_str, sac_val)) return;
                }

                if (is_wildcard(sic_str))
                {
                    any_sic = true;
                }
                else
                {
                    if (!parse_uint8(sic_str, sic_val)) return;
                }
            }

            m_has_patterns = true;

            if (any_sac && any_sic)
            {
                m_match_lut.set();
            }
            else if (any_sac)
            {
                for (size_t s = 0; s < 256; ++s)
                {
                    m_match_lut.set((s << 8) | sic_val);
                }
            }
            else if (any_sic)
            {
                const size_t base = static_cast<size_t>(sac_val) << 8;
                for (size_t i = 0; i < 256; ++i)
                {
                    m_match_lut.set(base | i);
                }
            }
            else
            {
                m_match_lut.set((static_cast<size_t>(sac_val) << 8) | sic_val);
            }
        };

        size_t start = 0;
        size_t end = current_str.find_first_of(";,");
        while (end != std::string::npos)
        {
            parse_token(current_str.substr(start, end - start));
            start = end + 1;
            end = current_str.find_first_of(";,", start);
        }
        parse_token(current_str.substr(start));
    }

    bool sac_sic_filter::handle_data(buffer*& buf)
    {
        if (!buf) return false;

        auto* root_frame = buf->begin_as<frame>();
        if (!root_frame) return false;

        adam::buffer* ref_buf = buf->get_referenced_buffer();
        const uint32_t data_size = (ref_buf && ref_buf->get_size() > 0) ? ref_buf->get_size() : buf->get_size();

        update_parsed_patterns();

        const bool is_whitelist = (m_filter_mode_param->get_value() == "whitelist"_ct);

        auto* stats = get_state_buffer_data();
        if (stats)
        {
            stats->total_buffers_recieved++;
            stats->total_bytes_recieved += data_size;
        }

        if (!m_has_patterns)
        {
            if (is_whitelist)
            {
                if (stats)
                {
                    stats->total_buffers_discarded++;
                    stats->total_bytes_discarded += data_size;
                }
                return false;
            }

            if (stats)
            {
                stats->total_buffers_forwarded++;
                stats->total_bytes_forwarded += data_size;
            }
            return true;
        }

        uint32_t total_records   = 0;
        uint32_t kept_records    = 0;
        uint32_t removed_records = 0;
        uint32_t kept_blocks     = 0;

        const uap* cached_base_uap = nullptr;
        uint8_t cached_cat = 0xFF;

        for (auto& blk : *root_frame)
        {
            if (blk.is_removed()) continue;

            if (blk.category != cached_cat)
            {
                cached_cat = blk.category;
                cached_base_uap = uap_pool::get().get_uap(cached_cat);
            }

            uint32_t records_in_block      = 0;
            uint32_t kept_records_in_block = 0;

            for (auto& rec : blk)
            {
                if (rec.is_removed()) continue;

                total_records++;
                records_in_block++;

                const uap* active_uap = nullptr;
                if (cached_base_uap)
                {
                    if (cached_base_uap->get_name() == rec.used_uap)
                    {
                        active_uap = cached_base_uap;
                    }
                    else
                    {
                        active_uap = cached_base_uap->find_alternative(rec.used_uap);
                    }
                }

                bool match = false;
                if (active_uap && m_has_patterns)
                {
                    const auto* raw = active_uap->get_record_sac_sic(&rec, ref_buf);
                    if (raw)
                    {
                        const size_t key = (static_cast<size_t>(raw->sac) << 8) | raw->sic;
                        match = m_match_lut.test(key);
                    }
                }

                const bool keep = is_whitelist ? match : !match;

                if (keep)
                {
                    kept_records++;
                    kept_records_in_block++;
                }
                else
                {
                    rec.set_removed(true);
                    removed_records++;
                }
            }

            if (kept_records_in_block == 0 && records_in_block > 0)
            {
                blk.set_removed(true);
            }
            else if (kept_records_in_block > 0)
            {
                kept_blocks++;
            }
        }

        if (kept_records == 0)
        {
            if (stats)
            {
                stats->total_buffers_discarded++;
                stats->total_bytes_discarded += data_size;
            }
            return false;
        }

        if (removed_records > 0)
        {
            root_frame->set_modified(true);
        }

        if (stats)
        {
            stats->total_buffers_forwarded++;
            stats->total_bytes_forwarded += data_size;
        }

        return true;
    }
}
