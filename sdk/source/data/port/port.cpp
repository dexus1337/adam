#include "data/port/port.hpp"


#include "data/format.hpp"
#include "data/inspector.hpp"
#include "memory/buffer/buffer-manager.hpp"


namespace adam 
{
    const configuration_parameter_list& port::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_boolean>("is_active"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("data_format"_ct));
            return p;
        }();
        return params;
    }

    port::~port() 
    {
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->destroy();
        });
    }

    bool port::handle_data(buffer* buffer)
    {
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->handle_data(buffer);
        });
        
        return true;
    }

    bool port::start()
    {
        m_statistic_buffer = buffer_manager::get().request_buffer(statistic_info_buffer_size);

        bool res = m_statistic_buffer != nullptr;

        m_is_active->set_value(res);

        return res;
    }

    bool port::stop()
    {
        if (m_statistic_buffer)
        {
            m_statistic_buffer->release();
            m_statistic_buffer = nullptr;
        }

        m_is_active->set_value(false);

        return true;
    }

    port::port(const string_hashed& item_name) 
    :   configuration_item(item_name, port::get_default_parameters()),
        m_data_format(&data_format_transparent),
        m_connections(),
        m_inspectors(),
        m_statistic_buffer(nullptr),
        m_is_active(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("is_active"_ct)))
    {
    }
}