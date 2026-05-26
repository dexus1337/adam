#include "data/connection.hpp"

#include "data/port/port.hpp"
#include "data/processor.hpp"
#include "controller/controller.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"


namespace adam 
{
    const configuration_parameter_list& connection::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_boolean>("is_active"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("inputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("processors"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("outputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_created"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_edited"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("color_code"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("sorting_index"_ct));
            return p;
        }();
        return params;
    }

    connection::connection(const string_hashed& item_name) 
    :   configuration_item(item_name, connection::get_default_parameters()),
        m_ports_input(),
        m_processors(),
        m_ports_output(),
        m_unavailable_inputs(),
        m_unavailable_outputs(),
        m_is_active(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("is_active"_ct)))
    {

    }

    connection::~connection() {}
    
    bool connection::handle_data(buffer* buffer)
    {
        bool result = true;

        // Chain processors: output of one = input of next
        m_processors.iterate([&](const auto& processors) 
        {
            for (auto* processor : processors) 
                result &= processor->handle_data(buffer);
        });

        // Send result to output ports
        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* output_port : outputs) 
                result &= output_port->handle_data(buffer, data_direction_out);
        });

        return result;
    }

    bool connection::start()
    {
        bool result = true;

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
            {
                if (!in->is_active())
                {
                    command cmd(command_type::port_start);
                    cmd.data_as<messages::port_action_data>()->port = in->get_name().get_hash();
                    controller::get().dispatcher().dispatch(&cmd, 1, *controller::get().get_context());
                }
            }
        });

        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* out : outputs) 
            {
                if (!out->is_active())
                {
                    command cmd(command_type::port_start);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    controller::get().dispatcher().dispatch(&cmd, 1, *controller::get().get_context());
                }
            }
        });

        m_is_active->set_value(result);

        return result;
    }

    bool connection::stop()
    {
        bool result = true;

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
            {
                bool is_used_elsewhere = false;
                in->connections().iterate([&](const auto& conns) 
                {
                    for (auto* c : conns) 
                    {
                        if (c != this && c->is_active()) 
                            is_used_elsewhere = true;
                    }
                });

                if (!is_used_elsewhere)
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = in->get_name().get_hash();
                    controller::get().dispatcher().dispatch(&cmd, 1, *controller::get().get_context());
                }
            }
        });

        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* out : outputs) 
            {
                bool is_used_elsewhere = false;
                out->connections().iterate([&](const auto& conns) 
                {
                    for (auto* c : conns) 
                    {
                        if (c != this && c->is_active()) 
                            is_used_elsewhere = true;
                    }
                });

                if (!is_used_elsewhere)
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    controller::get().dispatcher().dispatch(&cmd, 1, *controller::get().get_context());
                }
            }
        });

        if (result)
        {
            m_is_active->set_value(false);
        }

        return result;
    }
}