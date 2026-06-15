#include "module/internals/essential/data/filters/filter-frame-aligner.hpp"

#include "module/internals/essential/module-essential.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam 
{
    const configuration_parameter_list& filter_frame_aligner::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;

            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            auto byte_offset_param = std::make_unique<adam::configuration_parameter_integer>("byte_offset"_ct, 0);
            byte_offset_param->set_description(language_english, "The offset in bytes to skip from the beginning of the frame."_ct);
            byte_offset_param->set_description(language_german, "Der Offset in Bytes, der vom Anfang des Frames übersprungen werden soll."_ct);
            up->add(std::move(byte_offset_param));
            
            p.add(std::move(up));
            
            return p;
        }();
        return params;
    }

    filter_frame_aligner::filter_frame_aligner(const string_hashed& item_name) 
     :  filter(item_name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(internal_module_essential.get_name());

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);

        m_byte_offset = user_params->get<configuration_parameter_integer>("byte_offset"_ct);
        
        m_format_input  = &data_format_transparent;
        m_format_output = &data_format_transparent;
    }

    bool filter_frame_aligner::handle_data(buffer*& buf)
    {
        if (buf->get_size() < static_cast<uint32_t>(m_byte_offset->value()))
            return false;

        buf->move_start_pos(static_cast<uint32_t>(m_byte_offset->value()));

        return true;
    }
}