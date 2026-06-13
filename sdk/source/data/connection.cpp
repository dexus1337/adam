#include "data/connection.hpp"

#include "data/port.hpp"
#include "data/processor.hpp"
#include "data/format.hpp"
#include "data/inspector.hpp"
#include "data/encoder.hpp"
#include "controller/controller.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer.hpp"


namespace adam 
{
    const configuration_parameter_list& connection::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_boolean>("started"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("inputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list_sorted>("processors"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("outputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_created"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_edited"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("color_code"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("sorting_index"_ct));
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
        m_b_valid_data_chain(false),
        m_started(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("started"_ct)))
    {

    }

    connection::~connection() {}

    bool connection::handle_data(buffer* buf)
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
        if (m_output_format != &data_format_transparent)
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

        m_ports_input.iterate([&](const auto& inputs)
        {
            has_input = !inputs.empty();
        });

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
                const data_format* proc_in = proc->get_input_data_format();
                const data_format* proc_out = proc->get_output_data_format();

                if (*current_format != *proc_in)
                {
                    format_mismatch = true;
                    break;
                }

                current_format = proc_out;
            }
        });

        if (format_mismatch)
            return false;

        const data_format* expected_out = m_output_format;

        if (*current_format != *expected_out)
        {
            return false;
        }

        m_b_valid_data_chain = true;

        return true;
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
                if (!in->is_started())
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
                if (!out->is_started())
                {
                    command cmd(command_type::port_start);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    dispatch_command_safe(cmd);
                }
            }
        });

        m_started->set_value(result);

        return result;
    }

    bool connection::stop()
    {
        bool result = true;

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
                if (in->is_started() && !is_used_elsewhere(in))
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
                if (out->is_started() && !is_used_elsewhere(out))
                {
                    command cmd(command_type::port_stop);
                    cmd.data_as<messages::port_action_data>()->port = out->get_name().get_hash();
                    dispatch_command_safe(cmd);
                }
            }
        });

        if (result)
        {
            m_started->set_value(false);
        }

        return result;
    }
}