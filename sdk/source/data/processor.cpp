#include "data/processor.hpp"

#include "data/format.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "module/internals/essential/module-essential.hpp"

namespace adam 
{
    const configuration_parameter_list& processor::get_default_parameters()
    {
        static adam::configuration_parameter_list_sorted user_params = []() 
        {
            adam::configuration_parameter_list_sorted p;
            return p;
        }();

        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>(user_params);
            up->set_name("user_parameters"_ct);
            p.add(std::move(up));
            p.add(std::make_unique<adam::configuration_parameter_string>("type"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct));
            p.add(std::make_unique<adam::configuration_parameter_boolean>("is_filter"_ct));
            return p;
        }();
        
        return params;
    }

    processor::~processor()
    {
        m_state_buffer->release();
    };

    processor::processor(const string_hashed& item_name, uint32_t state_buffer_size)
     :  configuration_item(item_name, processor::get_default_parameters()),
        m_format_input(&data_format_transparent),
        m_format_output(&data_format_transparent)
    {
        m_state_buffer = buffer_manager::get().request_buffer(state_buffer_size);
    }
}