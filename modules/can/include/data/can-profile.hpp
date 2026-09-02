#pragma once

/**
 * @file    can-profile.hpp
 * @author  dexus1337
 * @brief   Defines CAN Profile, message, and 0-indexed signal structures for CAN decoding.
 * @version 1.0
 * @date    24.08.2026
 */

#include "api/api-can.hpp"
#include "types/byteswap.hpp"
#include <adam-core.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

namespace adam::modules::can
{
    /**
     * @struct can_signal_spec
     * @brief Defines a specific signal inside a CAN message (0-indexed).
     */
    struct can_signal_spec
    {
        uint8_t     index;          /**< 0-based Signal Index (like 0-indexed FRN). */
        const char* name;           /**< Signal Name (e.g. "RIZ_HL", "DHL"). */
        const char* description;    /**< Signal Description. */
        uint16_t    bit_offset;     /**< Bit offset in payload (0..63). */
        uint16_t    bit_length;     /**< Length in bits (1..64). */
    };

    /**
     * @struct can_message_spec
     * @brief Defines an ECU message containing an array of 0-indexed signals.
     */
    struct can_message_spec
    {
        uint32_t                can_id;         /**< Standard (0x000-0x7FF) or Extended ID. */
        bool                    is_extended;    /**< True if extended 29-bit CAN ID. */
        uint8_t                 dlc;            /**< Data Length Code. */
        const char*             ecu_name;       /**< ECU name (e.g. "EZS_A10"). */
        const char*             name;           /**< Message Description. */
        const can_signal_spec*  signals;        /**< Array of signals. */
        size_t                  signal_count;   /**< Number of signals in this message. */

        inline const can_signal_spec* get_signal(uint8_t index) const
        {
            if (index >= signal_count) return nullptr;
            return &signals[index];
        }
    };

    /**
     * @class can_profile
     * @brief Profile defining ECU messages and signal layouts for a specific CAN bus.
     */
    class ADAM_CAN_API can_profile
    {
    public:
        enum endianness : uint8_t
        {
            little_endian,
            big_endian
        };

        can_profile
        (
            const string_hashed_ct&     name,
            const string_hashed_ct&     description,
            const can_message_spec*     messages,
            size_t                      message_count,
            endianness                  endian
        );

        // Prevent copying
        can_profile(const can_profile&) = delete;
        can_profile& operator=(const can_profile&) = delete;

        inline const string_hashed_ct&                      get_name() const                            { return m_name; }
        inline const string_hashed_ct&                      get_description() const                     { return m_description; }
        inline endianness                                   get_endianness() const                      { return m_endianness; }
        inline const std::vector<const can_message_spec*>&  get_all_messages() const                    { return m_messages; }

        /** @brief O(1) direct lookup for standard 11-bit CAN IDs, and hash lookup for extended IDs */
        inline const can_message_spec*                      find_message(uint32_t can_id, bool is_extended) const
        {
            if (!is_extended && can_id < 2048) return m_std_id_table[can_id];

            auto it = m_extended_id_table.find(can_id);
            if (it != m_extended_id_table.end()) return it->second;

            return nullptr;
        }

        inline void                                         set_endianness(endianness endian)           { m_endianness = endian; }

    private:
        string_hashed_ct                                        m_name;
        string_hashed_ct                                        m_description;
        endianness                                              m_endianness;
        std::vector<const can_message_spec*>                    m_messages;
        const can_message_spec*                                 m_std_id_table[2048];
        std::unordered_map<uint32_t, const can_message_spec*>   m_extended_id_table;
    };

    /**
     * @brief Extracts a raw signal value (up to 64 bits) from a CAN payload.
     */
    inline uint64_t extract_raw_signal(const uint8_t* payload, uint8_t dlc, const can_signal_spec& spec, can_profile::endianness endian = can_profile::little_endian)
    {
        if (!payload || spec.bit_offset >= static_cast<uint16_t>(dlc * 8) || spec.bit_length == 0 || spec.bit_length > 64) return 0;

        if (endian == can_profile::little_endian)
        {
            uint64_t raw_buffer = 0;
            size_t copy_bytes = std::min<size_t>(dlc, sizeof(uint64_t));
            std::memcpy(&raw_buffer, payload, copy_bytes);

            if (spec.bit_offset >= 64) return 0;

            uint64_t shifted = raw_buffer >> spec.bit_offset;
            if (spec.bit_length >= 64) return shifted;

            uint64_t mask = (1ULL << spec.bit_length) - 1ULL;
            return shifted & mask;
        }

        // Big-endian (Motorola) extraction: most significant byte comes first
        size_t start_byte = spec.bit_offset / 8;
        size_t end_byte   = (spec.bit_offset + spec.bit_length - 1) / 8;

        if (end_byte >= dlc) end_byte = dlc - 1;
        if (start_byte > end_byte) return 0;

        uint64_t result = 0;
        for (size_t byte_idx = start_byte; byte_idx <= end_byte; ++byte_idx)
        {
            uint8_t bit_start_in_byte = (byte_idx == start_byte) ? static_cast<uint8_t>(spec.bit_offset % 8) : 0;
            uint8_t bit_end_in_byte   = (byte_idx == end_byte)   ? static_cast<uint8_t>((spec.bit_offset + spec.bit_length - 1) % 8) : 7;
            uint8_t bits_in_this_byte = bit_end_in_byte - bit_start_in_byte + 1;

            uint8_t byte_val = (payload[byte_idx] >> bit_start_in_byte) & ((1 << bits_in_this_byte) - 1);
            result = (result << bits_in_this_byte) | byte_val;
        }

        return result;
    }

    /**
     * @class can_profile_pool
     * @brief Global singleton registry for CAN profiles.
     */
    class ADAM_CAN_API can_profile_pool
    {
    public:
        static can_profile_pool& get();

        void register_profile(can_profile* profile);
        const can_profile* get_profile(string_hash name_hash) const;
        const can_profile* get_profile(const string_hashed& name) const;
        const can_profile* get_default_profile() const;
        const std::vector<const can_profile*>& get_profiles() const { return m_profiles; }

    private:
        can_profile_pool();
        ~can_profile_pool() = default;

        can_profile_pool(const can_profile_pool&) = delete;
        can_profile_pool& operator=(const can_profile_pool&) = delete;

        std::vector<const can_profile*>                         m_profiles;
        std::unordered_map<string_hash, const can_profile*>     m_profiles_by_hash;
    };
}
