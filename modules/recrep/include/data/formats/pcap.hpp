#pragma once

/**
 * @file    pcap.hpp
 * @author  dexus1337
 * @brief   Structures for the pcap data format.
 * @version 1.0
 * @date    04.06.2026
 */


#include "api/api-recrep.hpp"

#include <adam-sdk.hpp>
#include <cstdint>


namespace adam::modules::recrep
{
    namespace pcap
    {
        static const uint32_t magic_number = 0xa1b2c3d4;
        static const uint16_t version_major = 2;
        static const uint16_t version_minor = 4;

        enum network_type : uint32_t
        {
            ethernet = 1,
            loopback = 101,
            raw = 100,
        };

        #pragma pack(push, 1)
        struct file_header
        {
            uint32_t        magic_number;
            uint16_t        major_version;
            uint16_t        minor_version;
            uint32_t        thiszone;
            uint32_t        sigfigs;
            uint32_t        snaplen;
            network_type    network;
        };
        #pragma pack(pop)

        #pragma pack(push, 1)
        struct packet_header
        {
            uint32_t        ts_sec;
            uint32_t        ts_usec;
            uint32_t        incl_len;
            uint32_t        orig_len;
        };
        #pragma pack(pop)
    }
}