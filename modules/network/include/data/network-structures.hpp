#pragma once

/**
 * @file    network-structures.hpp
 * @author  dexus1337
 * @brief   Network header structures (IPv4, IPv6, TCP, UDP).
 * @version 1.0
 * @date    10.07.2026
 */

#include <cstdint>

#pragma pack(push, 1)

namespace adam::network
{
    struct ipv4_header;
    struct ipv6_header;

    struct ip_header
    {
        inline uint8_t version() const {  return (*reinterpret_cast<const uint8_t*>(this)); }

        inline ipv4_header* as_ipv4() { return reinterpret_cast<ipv4_header*>(this); }
        inline ipv6_header* as_ipv6() { return reinterpret_cast<ipv6_header*>(this); }
    };

    struct ipv4_header : ip_header
    {
        uint8_t  ihl_version; // version (4) and internet header length
        uint8_t  tos;         // type of service
        uint16_t tot_len;     // total length
        uint16_t id;          // identification
        uint16_t frag_off;    // fragment offset field
        uint8_t  ttl;         // time to live
        uint8_t  protocol;    // protocol
        uint16_t check;       // checksum
        uint32_t saddr;       // source address
        uint32_t daddr;       // destination address
    };

    struct ipv6_header : ip_header
    {
        uint32_t vtc_flow;    // version, traffic class, flow label
        uint16_t payload_len; // payload length
        uint8_t  next_header; // next header
        uint8_t  hop_limit;   // hop limit
        uint8_t  saddr[16];   // source address
        uint8_t  daddr[16];   // destination address
    };

    struct tcp_header
    {
        uint16_t source;      // source port
        uint16_t dest;        // destination port
        uint32_t seq;         // sequence number
        uint32_t ack_seq;     // acknowledgement number
        uint16_t res1_doff_flags; // data offset, res1, flags
        uint16_t window;      // window
        uint16_t check;       // checksum
        uint16_t urg_ptr;     // urgent pointer
    };

    struct udp_header
    {
        uint16_t source;      // source port
        uint16_t dest;        // destination port
        uint16_t len;         // length
        uint16_t check;       // checksum
    };
}

#pragma pack(pop)
