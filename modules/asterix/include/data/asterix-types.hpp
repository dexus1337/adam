#pragma once

/**
 * @file    asterix-types.hpp
 * @author  dexus1337
 * @brief   Defines the raw big-endian Asterix memory layout structures.
 * @version 1.0
 * @date    16.06.2026
 */

#include <api/api-sdk.hpp>
#include <cstdint>
#include <types/byteswap.hpp>
#include <iterator>

#include "api/api-asterix.hpp"
#include "data/asterix-internal.hpp"

namespace adam::modules::asterix
{
    ADAM_CONSTEXPR uint32_t min_block_length    = 5;
    ADAM_CONSTEXPR uint32_t highest_frn         = 256;
    ADAM_CONSTEXPR uint8_t  max_fspec_bytes     = highest_frn / 8;

    class uap;

    #pragma pack(push, 1)

    /**
     * @struct raw_fspec
     * @brief Represents one octet of the Field Specification (FSPEC) in Asterix raw wire format.
     */
    struct raw_fspec
    {
        uint8_t value;

        /**
         * @brief   Check if a general FRN is active in this variable-length FSPEC structure.
         *          CAUTION: O(n/7) Access
         * @param frn The Field Reference Number (e.g. 1, 26, 35).
         */
        inline bool is_frn_active(uint16_t frn) const
        {
            if (frn == 0) return false;
            uint32_t octet_index = (frn - 1) / 7;
            uint8_t bit_position = 7 - static_cast<uint8_t>((frn - 1) % 7);

            const raw_fspec* current = this;
            for (uint32_t i = 0; i < octet_index; ++i)
            {
                current = current->get_next();
                if (!current) return false;
            }
            return (current->value & (1 << bit_position)) != 0;
        }

        /** @brief Check if a local to this byte FRN is active. So only FRN 1-7 allowed. */
        inline bool is_frn_active_here(uint8_t frn) const { return value & (1 << (7 - ((frn - 1) % 7))); }

        /** @brief Check if the Field Extension indicator (FX) bit is set (bit 0). */
        inline bool has_extension() const { return (value & 1) != 0; }

        /** @brief Get the next raw_fspec octet if the Field Extension (FX) bit is set. */
        inline const raw_fspec* get_next() const { return has_extension() ? (this + 1) : nullptr; }

        /** @brief Counts the amount of bytes the whole fspec uses. */
        inline size_t size() const { size_t ret = 0; auto* start = this; while (start) { ret++; start = start->get_next(); } return ret; }

        /** @brief Forward iterator over the chain of raw_fspec octets.
         *
         * Usage:
         *   for (const auto& octet : fspec)          // *it  → raw_fspec&
         *       if (octet.has_extension()) { … }
         *
         * Or with an index:
         *   for (auto it = fspec.begin(); it != fspec.end(); ++it)
         *       process(it->value, it.byte_index());
         *
         * The iterator stops (reaches end) when the FX bit of the current
         * octet is 0, i.e. there is no next octet.
         */
        struct iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using value_type        = const raw_fspec;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const raw_fspec*;
            using reference         = const raw_fspec&;

            inline explicit iterator(const raw_fspec* p, uint16_t idx = 0)
                : m_ptr(p), m_byte_index(idx) {}

            /** @brief Dereference – gives direct access to the raw_fspec octet. */
            inline reference operator*()  const { return *m_ptr; }
            /** @brief Arrow – `it->value`, `it->has_extension()`, etc. */
            inline pointer   operator->() const { return  m_ptr; }

            /** @brief Zero-based index of the current octet within the FSPEC chain. */
            inline uint16_t byte_index() const { return m_byte_index; }

            inline iterator& operator++()
            {
                if (m_ptr)
                {
                    m_ptr = m_ptr->get_next();
                    ++m_byte_index;
                }
                return *this;
            }

            inline iterator operator++(int)
            {
                iterator temp = *this;
                ++(*this); // calls the upper ++ opeator on the local copy reference
                return temp;
            }

            inline bool operator==(const iterator& other) const { return m_ptr == other.m_ptr; }
            inline bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }

        private:
            const raw_fspec* m_ptr;
            uint16_t         m_byte_index;
        };

        inline iterator begin() const { return iterator(this, 0); }
        inline iterator end()   const { return iterator(nullptr, 0); }
    };

    /**
     * @struct raw_block_header
     * @brief Represents the raw big-endian block header at the start of an Asterix block.
     */
    struct raw_block_header
    {
        uint8_t  category;
        uint16_t length; // Big-endian

        inline uint16_t get_length() const
        {
            return adam::swap_2(reinterpret_cast<const uint8_t*>(&length));
        }

        inline const uap* get_uap() const; // defined in asterix-uap.hpp
    };

    /**
     * @struct raw_record_header
     * @brief Represents the raw record header at the start of an Asterix record.
     */
    struct raw_record_header
    {
        raw_fspec fspec; // at least 1 fspec
        
        inline const uap* retrieve_uap(const raw_block_header* blk, uint8_t fs, uint32_t raw_len) const; // defined in asterix-uap.hpp
    };

    /**
     * @struct raw_record_header
     * @brief Represents the raw big-endian record header at the start of an Asterix record.
     */
    struct raw_explicit_header
    {
        uint8_t item_count; // Number of items in the record (1-28)
    };

    /**
     * @struct raw_sac_sic
     * @brief Represents the raw System Area Code (SAC) and System Identification Code (SIC).
     */
    struct raw_sac_sic
    {
        uint8_t sac;
        uint8_t sic;

        inline uint8_t get_sac() const { return sac; }
        inline uint8_t get_sic() const { return sic; }
    };

    /**
     * @struct raw_cartesian_polar_coordinates
     * @brief Represents the raw Cartesian Polar Coordinates.
     */
    struct raw_cartesian_polar_coordinates
    {
        uint16_t rho_s_code;    /**< big-endian (1/256 NM) */
        uint16_t theta_s_code;  /**< big-endian (0.005 degrees) */

        inline double get_range() const { return adam::swap_2(reinterpret_cast<const uint8_t*>(&rho_s_code)) / 256.0; }
        inline double get_azimuth() const { return adam::swap_2(reinterpret_cast<const uint8_t*>(&theta_s_code)) * 0.005; }
    };

    #pragma pack(pop)
}
