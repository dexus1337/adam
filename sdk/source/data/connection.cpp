#include "data/connection.hpp"

#include "data/port.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
#include "data/inspector.hpp"
#include "data/encoder.hpp"
#include "module/module.hpp"
#include "controller/controller.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer.hpp"
#include "module/internals/essential/module-essential.hpp"

namespace adam 
{
    const configuration_parameter_list& connection::get_default_parameters()
    {
        static configuration_parameter_list params = []() 
        {
            configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_boolean>("started"_ct));
            p.add(std::make_unique<configuration_parameter_list>("inputs"_ct));
            p.add(std::make_unique<configuration_parameter_list_sorted>("processors"_ct));
            p.add(std::make_unique<configuration_parameter_list>("outputs"_ct));
            p.add(std::make_unique<configuration_parameter_integer>("date_created"_ct));
            p.add(std::make_unique<configuration_parameter_integer>("date_edited"_ct));
            p.add(std::make_unique<configuration_parameter_integer>("color_code"_ct));
            p.add(std::make_unique<configuration_parameter_integer>("sorting_index"_ct));
            p.add(std::make_unique<configuration_parameter_string>("input_format"_ct,           data_format_transparent.get_name()));
            p.add(std::make_unique<configuration_parameter_string>("input_format_module"_ct,    data_format_transparent.get_origin_module()->get_name()));
            p.add(std::make_unique<configuration_parameter_string>("output_format"_ct,          data_format_transparent.get_name()));
            p.add(std::make_unique<configuration_parameter_string>("output_format_module"_ct,   data_format_transparent.get_origin_module()->get_name()));
            return p;
        }();
        return params;
    }

    connection::connection(const string_hashed& item_name) 
    :   registry_item(item_name, connection::get_default_parameters()),
        m_ports_input(),
        m_processors(),
        m_ports_output(),
        m_unavailable_inputs(),
        m_unavailable_outputs(),
        m_input_format(&data_format_transparent),   // Default to transparent
        m_output_format(&data_format_transparent),  // Default to transparent
        m_b_valid_data_chain(false),
        m_started(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("started"_ct)))
    {

    }

    connection::~connection() {}

    void connection::set_input_format(const data_format* fmt)
    {
        m_input_format = fmt;
        check_valid_chain();
        m_ports_input.iterate([](const auto& inputs)
        {
            for (auto* in : inputs)
            {
                in->rebuild_formats_database();
            }
        });
    }

    void connection::set_output_format(const data_format* fmt)
    {
        m_output_format = fmt;
        check_valid_chain();
        m_ports_input.iterate([](const auto& inputs)
        {
            for (auto* in : inputs)
            {
                in->rebuild_formats_database();
            }
        });
    }

    bool connection::handle_data(buffer*& buf)
    {
        bool result = true;

        // Run data through input inspectors
        m_inspectors_input.iterate([&](const auto& inspectors) 
        {
            for (const auto& inspector : inspectors) 
                inspector->handle_data(buf);
        });

        // Run data through the processor chain
        m_processors.iterate([&](const auto& processors) 
        {
            for (auto* processor : processors) 
                result &= processor->handle_data(buf);
        });

        // Run data through output inspectors
        m_inspectors_output.iterate([&](const auto& inspectors) 
        {
            for (const auto& inspector : inspectors) 
                inspector->handle_data(buf);
        });

        // Run data through output encoder
        if (buf->get_data_format() != m_output_format && m_output_format->get_encoder())
        {
            buffer* encoded_buf = nullptr;
            m_output_format->get_encoder()->encode(encoded_buf, buf);

            buf = encoded_buf;
        }

        // Forward to all output ports
        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* output_port : outputs) 
                result &= output_port->handle_data(buf, data_direction_out);
        });

        return result;
    }

    bool connection::check_valid_chain()
    {
        m_b_valid_data_chain = false;

        bool has_input = false;
        bool has_output = false;

        // Force active input update
        m_ports_input.iterate([&](const auto& inputs)
        {
            has_input = !inputs.empty();
        });

        // Force active output update
        m_ports_output.iterate([&](const auto& outputs)
        {
            has_output = !outputs.empty();
        });

        if (!has_input || !has_output)
            return false;

        bool format_mismatch = false;
        const data_format* current_format = m_input_format;

        m_processors.iterate([&](const auto& processors)
        {
            for (auto* proc : processors)
            {
                const data_format* proc_in = proc->get_input_format();
                const data_format* proc_out = proc->get_output_format();

                if (current_format != proc_in)
                    format_mismatch = true;

                current_format = proc_out;
            }
        });

        if (format_mismatch)
            return false;

        const data_format* expected_out = m_output_format;

        if (current_format != expected_out)
            return false;

        m_b_valid_data_chain = true;

        return true;
    }

    bool connection::start()
    {
        bool result = true;
        auto* ctrl  = get_controller();

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
            {
                if (!in->is_running())
                {
                    command cmd(command_type::port_start);
                    cmd.data_as<messages::port_action_data>()->port = in->get_name().get_hash();
                    ctrl->dispatcher().dispatch(&cmd, 1, *ctrl->get_command_ctx());
                }
            }
        });

        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* out : outputs) 
            {
                if (!out->is_running())
                {
                    command cmd(command_type::port_start);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    ctrl->dispatcher().dispatch(&cmd, 1, *ctrl->get_command_ctx());
                }
            }
        });

        m_started->set_value(result);

        return result;
    }

    bool connection::stop()
    {
        bool result = true;
        auto* ctrl  = get_controller();

        auto is_used_elsewhere = [&](port* p)
        {
            bool is_used = false;
            p->get_in_connections().iterate([&](const auto& conns)
            {
                for (auto* c : conns)
                    if (c != this && c->is_started()) is_used = true;
            });
            if (is_used) return true;
            p->get_out_connections().iterate([&](const auto& conns)
            {
                for (auto* c : conns)
                    if (c != this && c->is_started()) is_used = true;
            });
            return is_used;
        };

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
            {
                if (in->is_running() && !is_used_elsewhere(in))
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = in->get_name().get_hash();
                    ctrl->dispatcher().dispatch(&cmd, 1, *ctrl->get_command_ctx());
                }
            }
        });

        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* out : outputs) 
            {
                if (out->is_running() && !is_used_elsewhere(out))
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    ctrl->dispatcher().dispatch(&cmd, 1, *ctrl->get_command_ctx());
                }
            }
        });

        m_started->set_value(false);
        
        return result;
    }
}