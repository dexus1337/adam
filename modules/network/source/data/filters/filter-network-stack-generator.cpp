#include "data/filters/filter-network-stack-generator.hpp"
#include "data/network-structures.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "memory/buffer/buffer.hpp"
#include <cstring>

#include <vector>
#include <string>
#include <sstream>

namespace adam::network
{
    static uint32_t parse_ipv4(const std::string& ip_str)
    {
        uint32_t ip = 0;
        std::stringstream ss(ip_str);
        std::string token;
        int shift = 24;
        while (std::getline(ss, token, '.') && shift >= 0)
        {
            uint32_t octet = 0;
            try { octet = std::stoul(token); } catch (...) {}
            ip |= (octet & 0xFF) << shift;
            shift -= 8;
        }
        return ip;
    }

    static void parse_ipv6(const std::string& ip_str, uint8_t* out)
    {
        // extremely simplified ipv6 parser for demo
        for (int i=0; i<16; ++i) out[i] = 0; 
    }

    const configuration_parameter_list& filter_network_stack_generator::get_user_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>("user_parameters"_ct);
            
            configuration_parameter_string::presets_container ip_presets;
            ip_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("ipv4"_ct, "ipv4"_ct));
            ip_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("ipv6"_ct, "ipv6"_ct));
            auto ip_version = std::make_unique<adam::configuration_parameter_string>("ip_version"_ct, "ipv4"_ct, std::move(ip_presets));
            ip_version->set_description(language_english, "The IP version to use (ipv4 or ipv6)."_ct);
            ip_version->set_description(language_german, "Die zu verwendende IP-Version (ipv4 oder ipv6)."_ct);

            configuration_parameter_string::presets_container protocol_presets;
            protocol_presets.emplace("0"_ct, std::make_unique<adam::configuration_parameter_string>("tcp"_ct, "tcp"_ct));
            protocol_presets.emplace("1"_ct, std::make_unique<adam::configuration_parameter_string>("udp"_ct, "udp"_ct));
            auto protocol = std::make_unique<adam::configuration_parameter_string>("protocol"_ct, "tcp"_ct, std::move(protocol_presets));
            protocol->set_description(language_english, "The transport protocol (tcp, udp, none)."_ct);
            protocol->set_description(language_german, "Das Transportprotokoll (tcp, udp, none)."_ct);
            
            auto src_ip = std::make_unique<adam::configuration_parameter_string>("src_ip"_ct, "127.0.0.1"_ct);
            src_ip->set_description(language_english, "The source IP address."_ct);
            src_ip->set_description(language_german, "Die Quell-IP-Adresse."_ct);
            
            auto dst_ip = std::make_unique<adam::configuration_parameter_string>("dst_ip"_ct, "127.0.0.1"_ct);
            dst_ip->set_description(language_english, "The destination IP address."_ct);
            dst_ip->set_description(language_german, "Die Ziel-IP-Adresse."_ct);
            
            auto src_port = std::make_unique<adam::configuration_parameter_integer>("src_port"_ct, 12345);
            src_port->set_description(language_english, "The source port number."_ct);
            src_port->set_description(language_german, "Die Quell-Portnummer."_ct);
            src_port->set_range(0, 65535);
            
            auto dst_port = std::make_unique<adam::configuration_parameter_integer>("dst_port"_ct, 80);
            dst_port->set_description(language_english, "The destination port number."_ct);
            dst_port->set_description(language_german, "Die Ziel-Portnummer."_ct);
            dst_port->set_range(0, 65535);
            
            auto ttl = std::make_unique<adam::configuration_parameter_integer>("ttl"_ct, 64);
            ttl->set_description(language_english, "Time To Live (TTL) value."_ct);
            ttl->set_description(language_german, "Time To Live (TTL) Wert."_ct);
            ttl->set_range(0, 255);
            
            auto tos = std::make_unique<adam::configuration_parameter_integer>("tos"_ct, 0);
            tos->set_description(language_english, "Type of Service (ToS) value."_ct);
            tos->set_description(language_german, "Type of Service (ToS) Wert."_ct);
            tos->set_range(0, 255);
            
            auto tcp_seq = std::make_unique<adam::configuration_parameter_integer>("tcp_seq"_ct, 0);
            tcp_seq->set_description(language_english, "TCP sequence number."_ct);
            tcp_seq->set_description(language_german, "TCP-Sequenznummer."_ct);
            tcp_seq->set_range(0, 4294967295LL);
            
            auto tcp_ack = std::make_unique<adam::configuration_parameter_integer>("tcp_ack"_ct, 0);
            tcp_ack->set_description(language_english, "TCP acknowledgment number."_ct);
            tcp_ack->set_description(language_german, "TCP-Bestätigungsnummer."_ct);
            tcp_ack->set_range(0, 4294967295LL);
            
            auto tcp_flags = std::make_unique<adam::configuration_parameter_integer>("tcp_flags"_ct, 2); // SYN by default
            tcp_flags->set_description(language_english, "TCP flags (e.g. 2 for SYN)."_ct);
            tcp_flags->set_description(language_german, "TCP-Flags (z.B. 2 für SYN)."_ct);
            tcp_flags->set_range(0, 255);

            up->add(std::move(ip_version));
            up->add(std::move(protocol));
            up->add(std::move(src_ip));
            up->add(std::move(dst_ip));
            up->add(std::move(src_port));
            up->add(std::move(dst_port));
            up->add(std::move(ttl));
            up->add(std::move(tos));
            up->add(std::move(tcp_seq));
            up->add(std::move(tcp_ack));
            up->add(std::move(tcp_flags));
            
            p.add(std::move(up));
            return p;
        }();
        return params;
    }

    filter_network_stack_generator::filter_network_stack_generator(const string_hashed& item_name) 
     :  filter(item_name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value("network"_ct);

        add_parameters(get_user_parameters());

        auto* user_params = get_parameter<configuration_parameter_list_sorted>("user_parameters"_ct);

        m_ip_version = user_params->get<configuration_parameter_string>("ip_version"_ct);
        m_protocol = user_params->get<configuration_parameter_string>("protocol"_ct);
        
        m_src_ip = user_params->get<configuration_parameter_string>("src_ip"_ct);
        m_dst_ip = user_params->get<configuration_parameter_string>("dst_ip"_ct);
        m_src_port = user_params->get<configuration_parameter_integer>("src_port"_ct);
        m_dst_port = user_params->get<configuration_parameter_integer>("dst_port"_ct);
        
        m_ttl = user_params->get<configuration_parameter_integer>("ttl"_ct);
        m_tos = user_params->get<configuration_parameter_integer>("tos"_ct);
        
        m_tcp_seq = user_params->get<configuration_parameter_integer>("tcp_seq"_ct);
        m_tcp_ack = user_params->get<configuration_parameter_integer>("tcp_ack"_ct);
        m_tcp_flags = user_params->get<configuration_parameter_integer>("tcp_flags"_ct);

        m_format_input  = &data_format_transparent;
        m_format_output = &data_format_transparent;
    }

    bool filter_network_stack_generator::handle_data(buffer*& buf)
    {
        auto* stats = get_state_buffer_data();
        stats->total_buffers_recieved++;
        stats->total_bytes_recieved += buf->get_size();

        uint32_t initial_size = buf->get_size();

        const string_hashed& ip_ver = m_ip_version->get_value();
        const string_hashed& proto = m_protocol->get_value();
        
        uint32_t transport_len = 0;
        uint8_t protocol_id = 0;
        
        if (proto == "tcp"_ct) transport_len = sizeof(tcp_header);
        else if (proto == "udp"_ct) transport_len = sizeof(udp_header);
        
        uint32_t ip_len = 0;
        if (ip_ver == "ipv4"_ct) ip_len = sizeof(ipv4_header);
        else if (ip_ver == "ipv6"_ct) ip_len = sizeof(ipv6_header);
        else
        {
            stats->total_buffers_discarded++;
            stats->total_bytes_discarded += initial_size;
            return false;
        }
        
        uint32_t total_header_len = transport_len + ip_len;
        
        if (buf->get_start_pos() < total_header_len)
        {
            if (buf->get_size() + total_header_len <= buf->get_capacity())
            {
                uint32_t shift = total_header_len - buf->get_start_pos();
                std::memmove(buf->data_as<uint8_t>() + buf->get_start_pos() + shift, buf->begin(), buf->get_size());
                buf->set_start_pos(total_header_len);
            }
            else
            {
                buffer* new_buf = buffer_manager::get().request_buffer(buf->get_size() + total_header_len);
                if (!new_buf)
                {
                    stats->total_buffers_discarded++;
                    stats->total_bytes_discarded += initial_size;
                    return false;
                }
                
                new_buf->set_start_pos(total_header_len);
                new_buf->set_size(buf->get_size());
                std::memcpy(new_buf->begin(), buf->begin(), buf->get_size());
                
                new_buf->set_timestamp(buf->get_timestamp());
                new_buf->set_data_format(buf->get_data_format());
                if (buf->get_referenced_buffer()) new_buf->set_referenced_buffer(buf->get_referenced_buffer());
                
                buf->release();
                buf = new_buf;
            }
        }
        
        // 1. Generate Transport Header
        switch (proto)
        {
            case "tcp"_ct:
            {
                transport_len = sizeof(tcp_header);
                buf->move_start_pos(-static_cast<int32_t>(transport_len));
                
                tcp_header* tcph = buf->begin_as<tcp_header>();
                
                uint16_t src = static_cast<uint16_t>(m_src_port->value());
                uint16_t dst = static_cast<uint16_t>(m_dst_port->value());
                tcph->source = (src << 8) | (src >> 8);
                tcph->dest = (dst << 8) | (dst >> 8);
                
                tcph->seq = static_cast<uint32_t>(m_tcp_seq->value());
                tcph->ack_seq = static_cast<uint32_t>(m_tcp_ack->value());
                
                uint16_t data_offset = 5; // 20 bytes
                uint16_t flags = static_cast<uint16_t>(m_tcp_flags->value());
                
                uint16_t res1_doff_flags = (data_offset << 12) | (flags & 0x01FF);
                tcph->res1_doff_flags = (res1_doff_flags << 8) | (res1_doff_flags >> 8);
                
                tcph->window = 0;
                tcph->check = 0;
                tcph->urg_ptr = 0;
                
                protocol_id = 6;
                break;
            }
            case "udp"_ct:
            {
                transport_len = sizeof(udp_header);
                buf->move_start_pos(-static_cast<int32_t>(transport_len));
                
                udp_header* udph = buf->begin_as<udp_header>();
                
                uint16_t src = static_cast<uint16_t>(m_src_port->value());
                uint16_t dst = static_cast<uint16_t>(m_dst_port->value());
                udph->source = (src << 8) | (src >> 8);
                udph->dest = (dst << 8) | (dst >> 8);
                
                uint16_t payload_len = static_cast<uint16_t>(buf->get_size());
                udph->len = (payload_len << 8) | (payload_len >> 8);
                udph->check = 0;
                
                protocol_id = 17;
                break;
            }
        }
        
        // 2. Generate IP Header
        switch (ip_ver)
        {
            case "ipv4"_ct:
            {
                uint32_t ip_len = sizeof(ipv4_header);
                buf->move_start_pos(-static_cast<int32_t>(ip_len));
                
                ipv4_header* iph = buf->begin_as<ipv4_header>();
                
                iph->ihl_version = (4 << 4) | 5; // IPv4, 5 * 4 = 20 bytes
                iph->tos = static_cast<uint8_t>(m_tos->value());
                iph->tot_len = 0; 
                iph->id = 0;
                iph->frag_off = 0;
                iph->ttl = static_cast<uint8_t>(m_ttl->value());
                iph->protocol = protocol_id;
                iph->check = 0;
                
                uint32_t saddr = parse_ipv4(m_src_ip->get_value().c_str());
                uint32_t daddr = parse_ipv4(m_dst_ip->get_value().c_str());
                
                iph->saddr = ((saddr >> 24) & 0xff) | ((saddr << 8) & 0xff0000) | ((saddr >> 8) & 0xff00) | ((saddr << 24) & 0xff000000);
                iph->daddr = ((daddr >> 24) & 0xff) | ((daddr << 8) & 0xff0000) | ((daddr >> 8) & 0xff00) | ((daddr << 24) & 0xff000000);
                break;
            }
            case "ipv6"_ct:
            {
                uint32_t ip_len = sizeof(ipv6_header);
                buf->move_start_pos(-static_cast<int32_t>(ip_len));
                
                ipv6_header* iph = buf->begin_as<ipv6_header>();
                
                iph->vtc_flow = (6 << 28); 
                iph->payload_len = 0; 
                iph->next_header = protocol_id;
                iph->hop_limit = static_cast<uint8_t>(m_ttl->value());
                
                parse_ipv6(m_src_ip->get_value().c_str(), iph->saddr);
                parse_ipv6(m_dst_ip->get_value().c_str(), iph->daddr);
                break;
            }
        }

        stats->total_buffers_forwarded++;
        stats->total_bytes_forwarded += buf->get_size();

        return true;
    }
}
