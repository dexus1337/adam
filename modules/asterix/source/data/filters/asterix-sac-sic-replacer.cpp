#include "data/filters/asterix-sac-sic-replacer.hpp"

#include "data/format-asterix.hpp"
#include "module/internals/essential/module-essential.hpp"
#include "module/module-asterix.hpp"
#include "data/asterix-internal.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include <memory>
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam::modules::asterix
{
    const configuration_parameter_list& sac_sic_replacer::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto sac_param = std::make_unique<configuration_parameter_integer>("sac_value"_ct, 0, 0, 255);
            sac_param->set_description(language_english, "The new System Area Code (SAC) to inject."_ct);
            sac_param->set_description(language_german, "Der neue System Area Code (SAC) zum Einfügen."_ct);
            up->add(std::move(sac_param));

            auto sic_param = std::make_unique<configuration_parameter_integer>("sic_value"_ct, 0, 0, 255);
            sic_param->set_description(language_english, "The new System Identification Code (SIC) to inject."_ct);
            sic_param->set_description(language_german, "Der neue System Identification Code (SIC) zum Einfügen."_ct);
            up->add(std::move(sic_param));
            
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
        m_sac_param = user_params->get<configuration_parameter_integer>("sac_value"_ct);
        m_sic_param = user_params->get<configuration_parameter_integer>("sic_value"_ct);
    }

    bool sac_sic_replacer::handle_data(buffer*& buf)
    {
        if (!buf) return false;

        const auto* root_frame = reinterpret_cast<const frame*>(buf->get_data());
        adam::buffer* ref_buf = buf->get_referenced_buffer();
        
        if (!ref_buf) return false;

        uint8_t new_sac = static_cast<uint8_t>(m_sac_param->get_value());
        uint8_t new_sic = static_cast<uint8_t>(m_sic_param->get_value());

        // Iterate over blocks
        for (auto block_it = root_frame->begin(); block_it != root_frame->end(); ++block_it)
        {
            // Iterate over records in the block
            for (auto rec_it = block_it->begin(); rec_it != block_it->end(); ++rec_it)
            {
                const auto* used_uap = rec_it->find_used_uap();

                if (!used_uap || !used_uap->get_sacsic_frn())
                    continue;

                const item* sac_sic_item = rec_it->get_item(used_uap->get_sacsic_frn());
                if (sac_sic_item && sac_sic_item->is_populated())
                {
                    raw_sac_sic* sac_sic = ref_buf->at<raw_sac_sic>(sac_sic_item->raw_offset);
                    
                    sac_sic->sac = new_sac;
                    sac_sic->sic = new_sic;
                }
            }
        }

        return true;
    }
}
