#include "data/filters/asterix-sac-sic-replacer.hpp"

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
    const configuration_parameter_list& sac_sic_replacer::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto source_param = std::make_unique<configuration_parameter_string>("source_sac_sic"_ct, ""_ct);
            source_param->set_description(language_english, "Semicolon or comma separated list of source SAC/SIC patterns to match (e.g. 103/x; x/63; 103/200; x/x)."_ct);
            source_param->set_description(language_german, "Durch Semikolon oder Komma getrennte Liste von Quell-SAC/SIC-Mustern (z.B. 103/x; x/63; 103/200; x/x)."_ct);
            up->add(std::move(source_param));

            auto target_param = std::make_unique<configuration_parameter_string>("target_sac_sic"_ct, "x/x"_ct);
            target_param->set_description(language_english, "Target SAC/SIC to inject (e.g. 105/x; x/200; 105/200; x/x). 'x' preserves the original value."_ct);
            target_param->set_description(language_german, "Ziel-SAC/SIC zum Einfügen (z.B. 105/x; x/200; 105/200; x/x). 'x' behält den Originalwert bei."_ct);
            up->add(std::move(target_param));
            
            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    sac_sic_replacer::sac_sic_replacer(const string_hashed& name) : filter(name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(get_adam_module()->get_name());

        m_format_input  = &data_format_asterix;
        m_format_output = &data_format_asterix;

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);
        m_source_param = user_params->get<configuration_parameter_string>("source_sac_sic"_ct);
        m_target_param = user_params->get<configuration_parameter_string>("target_sac_sic"_ct);

        for (size_t i = 0; i < 0xffff; ++i)
        {
            m_replacement_lut[i] = { static_cast<uint8_t>(i >> 8), static_cast<uint8_t>(i & 0xFF), false };
        }
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

    void sac_sic_replacer::update_parsed_patterns()
    {
        if (m_last_source_hash == m_source_param->get_value() && m_last_target_hash == m_target_param->get_value()) return;

        m_last_source_hash = m_source_param->get_value();
        m_last_target_hash = m_target_param->get_value();

        std::string source_str = std::string(m_last_source_hash);
        std::string target_str = std::string(m_last_target_hash);

        std::bitset<0xffff> source_match_lut;
        source_match_lut.reset();
        m_has_source_patterns = false;

        auto parse_source_token = [&source_match_lut, this](const std::string& token)
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

            m_has_source_patterns = true;

            if (any_sac && any_sic)
            {
                source_match_lut.set();
            }
            else if (any_sac)
            {
                for (size_t s = 0; s < 256; ++s)
                {
                    source_match_lut.set((s << 8) | sic_val);
                }
            }
            else if (any_sic)
            {
                const size_t base = static_cast<size_t>(sac_val) << 8;
                for (size_t i = 0; i < 256; ++i)
                {
                    source_match_lut.set(base | i);
                }
            }
            else
            {
                source_match_lut.set((static_cast<size_t>(sac_val) << 8) | sic_val);
            }
        };

        size_t start = 0;
        size_t end = source_str.find_first_of(";,");
        while (end != std::string::npos)
        {
            parse_source_token(source_str.substr(start, end - start));
            start = end + 1;
            end = source_str.find_first_of(";,", start);
        }
        parse_source_token(source_str.substr(start));

        // Parse target pattern
        bool target_any_sac = true;
        bool target_any_sic = true;
        uint8_t target_sac = 0;
        uint8_t target_sic = 0;

        std::string trimmed_target = trim_string(target_str);
        if (!trimmed_target.empty())
        {
            size_t slash_pos = trimmed_target.find_first_of("/:\\");
            if (slash_pos == std::string::npos)
            {
                if (!is_wildcard(trimmed_target))
                {
                    if (parse_uint8(trimmed_target, target_sac))
                    {
                        target_any_sac = false;
                    }
                }
            }
            else
            {
                std::string t_sac_str = trim_string(trimmed_target.substr(0, slash_pos));
                std::string t_sic_str = trim_string(trimmed_target.substr(slash_pos + 1));

                if (!is_wildcard(t_sac_str))
                {
                    if (parse_uint8(t_sac_str, target_sac))
                    {
                        target_any_sac = false;
                    }
                }

                if (!is_wildcard(t_sic_str))
                {
                    if (parse_uint8(t_sic_str, target_sic))
                    {
                        target_any_sic = false;
                    }
                }
            }
        }

        // Build full 65,536-entry replacement LUT
        for (size_t s = 0; s < 256; ++s)
        {
            for (size_t i = 0; i < 256; ++i)
            {
                const size_t key = (s << 8) | i;
                if (m_has_source_patterns && source_match_lut.test(key))
                {
                    uint8_t new_sac = target_any_sac ? static_cast<uint8_t>(s) : target_sac;
                    uint8_t new_sic = target_any_sic ? static_cast<uint8_t>(i) : target_sic;
                    m_replacement_lut[key] = { new_sac, new_sic, (new_sac != s || new_sic != i) };
                }
                else
                {
                    m_replacement_lut[key] = { static_cast<uint8_t>(s), static_cast<uint8_t>(i), false };
                }
            }
        }
    }

    bool sac_sic_replacer::handle_data(buffer*& buf)
    {
        if (!buf) return false;

        auto* root_frame = buf->begin_as<frame>();
        if (!root_frame) return false;

        adam::buffer* ref_buf = buf->get_referenced_buffer();
        const uint32_t data_size = (ref_buf && ref_buf->get_size() > 0) ? ref_buf->get_size() : buf->get_size();

        update_parsed_patterns();

        auto* stats = get_state_buffer_data();
        if (stats)
        {
            stats->total_buffers_recieved++;
            stats->total_bytes_recieved += data_size;
        }

        if (!m_has_source_patterns)
        {
            if (stats)
            {
                stats->total_buffers_forwarded++;
                stats->total_bytes_forwarded += data_size;
            }
            return true;
        }

        bool frame_modified = false;
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

            for (auto& rec : blk)
            {
                if (rec.is_removed()) continue;

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

                if (active_uap && m_has_source_patterns)
                {
                    const auto* const_raw = active_uap->get_record_sac_sic(&rec, ref_buf);
                    if (const_raw)
                    {
                        const size_t key = (static_cast<size_t>(const_raw->sac) << 8) | const_raw->sic;
                        const auto& rep = m_replacement_lut[key];
                        if (rep.changed)
                        {
                            auto* raw = const_cast<raw_sac_sic*>(const_raw);
                            raw->sac = rep.new_sac;
                            raw->sic = rep.new_sic;
                            frame_modified = true;
                        }
                    }
                }
            }
        }

        if (frame_modified)
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
