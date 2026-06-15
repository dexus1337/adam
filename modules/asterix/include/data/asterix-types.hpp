#pragma once

/**
 * @file    asterix-types.hpp
 * @author  dexus1337
 * @brief   Defines the internal zero-copy POD structures for the asterix format.
 * @version 1.0
 * @date    15.06.2026
 */

#include "api/api-asterix.hpp"
#include <cstdint>

namespace adam::modules::asterix::data
{
    /**
     * @enum asterix_item_type
     * @brief The type of an asterix item.
     */
    enum class asterix_item_type : uint8_t
    {
        FIXED,
        VARIABLE,
        REPETITIVE,
        COMPOUND,
        EXPLICIT
    };

    #pragma pack(push, 1)

    /**
     * @struct asterix_item_header
     * @brief The common header for all parsed asterix items within the buffer.
     */
    struct asterix_item_header
    {
        uint32_t          raw_data_offset;  /**< Offset to the raw byte data in the referenced source buffer. */
        uint16_t          data_length;      /**< Length of the data in bytes. */
        asterix_item_type type;             /**< Type of the item. */
        uint8_t           flags;            /**< Additional flags or FSPEC metadata. */

        inline asterix_item_header(uint32_t raw_offset, uint16_t len, asterix_item_type type, uint8_t flags = 0)
            : raw_data_offset(raw_offset), data_length(len), type(type), flags(flags) {}
    };

    /**
     * @struct asterix_repetitive_item
     * @brief Represents a repetitive item. It is immediately followed in memory by `child_count` uint32_t offsets.
     */
    struct asterix_repetitive_item
    {
        asterix_item_header header;
        uint16_t            child_count;

        inline asterix_repetitive_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, asterix_item_type::REPETITIVE), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(asterix_repetitive_item));
        }
        
        inline const uint32_t* get_child_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(asterix_repetitive_item));
        }
    };

    /**
     * @struct asterix_compound_item
     * @brief Represents a compound item. It is immediately followed in memory by `child_count` uint32_t offsets.
     */
    struct asterix_compound_item
    {
        asterix_item_header header;
        uint16_t            child_count; // Number of items actually present based on FSPEC

        inline asterix_compound_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, asterix_item_type::COMPOUND), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(asterix_compound_item));
        }
        
        inline const uint32_t* get_child_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(asterix_compound_item));
        }
    };

    /**
     * @struct asterix_explicit_item
     * @brief Represents an explicit item, which might have its own children (recursive).
     */
    struct asterix_explicit_item
    {
        asterix_item_header header;
        uint16_t            child_count;

        inline asterix_explicit_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, asterix_item_type::EXPLICIT), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(asterix_explicit_item));
        }
        
        inline const uint32_t* get_child_offsets() const
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(asterix_explicit_item));
        }
    };

    #pragma pack(pop)
}
