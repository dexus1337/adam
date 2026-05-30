#include "data/connection.hpp"

#include "data/port/port.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
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
            // Input and output formats stored as string params for persistence
            p.add(std::make_unique<adam::configuration_parameter_string>("input_format"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("input_format_module"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("output_format"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("output_format_module"_ct));
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
        m_input_format(&data_format_transparent),   // Default to transparent
        m_output_format(&data_format_transparent),  // Default to transparent
        m_is_active(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("is_active"_ct)))
    {

    }

    connection::~connection() {}

    bool connection::handle_data(buffer* buffer, port* input_port)
    {
        bool result = true;

        // Run data through the processor chain
        m_processors.iterate([&](const auto& processors) 
        {
            for (auto* processor : processors) 
                result &= processor->handle_data(buffer);
        });

        // Forward to all output ports
        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* output_port : outputs) 
                result &= output_port->handle_data(buffer, data_direction_out);
        });

        (void)input_port; // No per-port compatibility check needed

        return result;
    }

    bool connection::has_valid_chain()
    {
        // A valid chain simply requires at least one input and one output port
        bool has_input = false;
        bool has_output = false;

        m_ports_input.iterate([&](const auto& inputs)
        {
            has_input = !inputs.empty();
        });

        m_ports_output.iterate([&](const auto& outputs)
        {
            has_output = !outputs.empty();
        });

        return has_input && has_output;
    }

    namespace
    {
        void dispatch_command_safe(const command& cmd)
        {
            auto* active_context = controller::get().get_context();
            if (active_context)
            {
                controller::get().dispatcher().dispatch(&cmd, 1, *active_context);
            }
            else
            {
                std::vector<response> dummy_resps(1);
                command_context dummy_ctx { os::get_current_thread_id(), controller::get().get_registry(), controller::get(), dummy_resps, {} };
                controller::get().dispatcher().dispatch(&cmd, 1, dummy_ctx);
            }
        }
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
                    dispatch_command_safe(cmd);
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
                    dispatch_command_safe(cmd);
                }
            }
        });

        m_is_active->set_value(result);

        return result;
    }

    bool connection::stop()
    {
        bool result = true;

        auto is_used_elsewhere = [&](port* p)
        {
            bool is_used = false;
            p->in_connections().iterate([&](const auto& conns)
            {
                for (auto* c : conns)
                    if (c != this && c->is_active()) is_used = true;
            });
            if (is_used) return true;
            p->out_connections().iterate([&](const auto& conns)
            {
                for (auto* c : conns)
                    if (c != this && c->is_active()) is_used = true;
            });
            return is_used;
        };

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
            {
                if (!is_used_elsewhere(in))
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = in->get_name().get_hash();
                    dispatch_command_safe(cmd);
                }
            }
        });

        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* out : outputs) 
            {
                if (!is_used_elsewhere(out))
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    dispatch_command_safe(cmd);
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