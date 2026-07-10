#include "data/filters/filter-network-stack-remover.hpp"
#include "data/network-structures.hpp"

// Assuming internal network module definition exists elsewhere,
// we will just define it locally or use the module name dynamically if available.
// For now, we will use the string literal "network" or similar.

namespace adam::network
{
    const configuration_parameter_list& filter_network_stack_remover::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            return p; // No user parameters needed
        }();
        return params;
    }

    filter_network_stack_remover::filter_network_stack_remover(const string_hashed& item_name) 
     :  filter(item_name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value("network"_ct);

        add_parameters(get_user_parameters());

        m_format_input  = &data_format_transparent;
        m_format_output = &data_format_transparent;
    }

    bool filter_network_stack_remover::handle_data(buffer*& buf)
    {
        if (buf->get_size() < 20) // Minimum IPv4 header size
            return false;

        ip_header* iph = buf->begin_as<ip_header>();
        uint8_t version = iph->version();
        
        uint32_t ip_header_len = 0;
        uint8_t protocol = 0;

        if (version == 4)
        {
            ipv4_header* v4 = iph->as_ipv4();
            ip_header_len = (v4->ihl_version & 0x0F) * 4;
            if (buf->get_size() < ip_header_len) return false;
            protocol = v4->protocol;
        }
        else if (version == 6)
        {
            ipv6_header* v6 = iph->as_ipv6();
            ip_header_len = 40; // Basic IPv6 header
            if (buf->get_size() < ip_header_len) return false;
            protocol = v6->next_header;
            // Note: Does not handle IPv6 extension headers currently.
        }
        else
        {
            return false; // Unknown IP version
        }

        uint32_t transport_header_len = 0;

        if (protocol == 6) // TCP
        {
            if (buf->get_size() < ip_header_len + 20) return false;
            tcp_header* tcph = reinterpret_cast<tcp_header*>(buf->begin_as<uint8_t>() + ip_header_len);
            // res1_doff_flags is in network byte order, but we can just read the byte directly
            // data offset is the top 4 bits of the 13th byte (which is offset 12 in the TCP header)
            uint8_t* tcp_bytes = reinterpret_cast<uint8_t*>(tcph);
            uint8_t data_offset = tcp_bytes[12] >> 4;
            transport_header_len = data_offset * 4;
            if (transport_header_len < 20) return false;
        }
        else if (protocol == 17) // UDP
        {
            transport_header_len = 8;
        }
        else
        {
            return false; // Not TCP or UDP
        }

        uint32_t total_header_len = ip_header_len + transport_header_len;
        if (buf->get_size() < total_header_len) return false;

        buf->move_start_pos(total_header_len);
        return true;
    }
}
