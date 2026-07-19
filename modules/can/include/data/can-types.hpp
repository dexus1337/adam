#pragma once

/**
 * @file    can-types.hpp
 * @author  dexus1337
 * @brief   Defines the raw big-endian Asterix memory layout structures.
 * @version 1.0
 * @date    16.06.2026
 */

#include <api/api-sdk.hpp>
#include <cstdint>
#include <types/byteswap.hpp>

#include "api/api-can.hpp"

namespace adam::modules::can
{
    #pragma pack(push, 1)

    /**
    * @brief Structure representing a CAN message frame.
    */
    struct can_message
    {
        union
        {
            uint32_t raw;                 ///< Raw 32-bit value containing ID and flags
            struct
            {
                uint32_t std : 11;        ///< 11-bit standard ID
            } std_bits;
            struct
            {
                uint32_t ext : 29;        ///< 29-bit extended ID
            } ext_bits;
            struct
            {
                uint32_t r1 : 1;          ///< Physical CAN reserved bit (LSB, bit 0, transmitted last)
                uint32_t ext_id : 18;     ///< Extended 18-bit ID portion (bits 1-18)
                uint32_t is_extended : 1; ///< IDE flag (bit 19, 0 = Standard, 1 = Extended)
                uint32_t rtr : 1;         ///< RTR flag (bit 20)
                uint32_t std_id : 11;     ///< Standard 11-bit ID portion (MSB, bits 21-31, transmitted first)
            } bits;
        } id;
        uint8_t dlc;                      ///< Data Length Code (0-8 bytes)
        uint8_t data[8];                  ///< Payload bytes (only the first dlc bytes are valid)

        /**
        * @brief Gets the full CAN ID.
        * 
        * @return The 11-bit standard ID or the reconstructed 29-bit extended ID.
        */
        inline uint32_t get_id() const
        {
            if (id.bits.is_extended)
            {
                return (id.bits.std_id << 18) | id.bits.ext_id;
            }
            return id.bits.std_id;
        }
    };

    #pragma pack(pop)
}
