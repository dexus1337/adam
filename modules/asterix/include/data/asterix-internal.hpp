#pragma once

/**
 * @file    asterix-internal.hpp
 * @author  dexus1337
 * @brief   Defines the internal parsed zero-copy tree nodes and headers for Asterix.
 * @version 1.0
 * @date    16.06.2026
 */

#include "api/api-asterix.hpp"
#include <cstdint>
#include <types/byteswap.hpp>

namespace adam::modules::asterix
{
    /**
     * @enum item_type
     * @brief The type of an asterix item.
     */
    enum item_type : uint8_t
    {
        item_type_fixed,
        item_type_variable,
        item_type_repetetive,
        item_type_compound,
        item_type_explicit
    };

    #pragma pack(push, 1)

    /**
     * @struct frame
     * @brief Header at the very start of the parsed internal_data buffer.
     * Followed in memory by `block_count` offsets to blocks (uint32_t).
     */
    struct frame
    {
        uint16_t block_count;

        inline frame(uint16_t blocks) : block_count(blocks) {}

        inline uint32_t* get_block_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(frame));
        }
        
        inline const uint32_t* get_block_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(frame));
        }
    };

    /**
     * @struct block
     * @brief Represents a parsed Asterix block.
     * Followed in memory by `record_count` offsets to records (uint32_t relative offsets from start of internal_data).
     */
    struct block
    {
        uint8_t  category;      /**< Asterix category (e.g. 48, 62). */
        uint16_t block_length;  /**< Length of raw data for this block. */
        uint16_t record_count;  /**< Number of records in this block. */
        uint32_t raw_offset;    /**< Offset in raw buffer where this block starts. */

        inline block(uint8_t category, uint16_t record_count, uint32_t len, uint32_t raw_off) : category(category), record_count(record_count), block_length(static_cast<uint16_t>(len)), raw_offset(raw_off) {}

        inline uint32_t* get_record_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(block));
        }
        
        inline const uint32_t* get_record_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(block));
        }
    };

    /**
     * @struct record
     * @brief Represents a parsed Asterix record within a block.
     * Followed in memory by `item_count` offsets to items (uint32_t relative offsets from start of internal_data).
     */
    struct record
    {
        uint16_t item_count;    /**< Number of items in this record. */
        uint32_t record_length; /**< Length of raw data for this record. */
        uint32_t raw_offset;    /**< Offset in raw buffer where this record starts. */

        inline record(uint16_t items, uint32_t len, uint32_t raw_off) : item_count(items), record_length(len), raw_offset(raw_off) {}

        inline uint32_t* get_item_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(record));
        }
        
        inline const uint32_t* get_item_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record));
        }
    };

    /**
     * @struct item
     * @brief The common header for all parsed asterix items within the buffer.
     */
    struct item
    {
        uint32_t    raw_data_offset;  /**< Offset to the raw byte data in the referenced source buffer. */
        uint16_t    data_length;      /**< Length of the data in bytes. */
        item_type   type;             /**< Type of the item. */
        uint8_t     flags;            /**< Additional flags or FSPEC metadata. */

        inline item(uint32_t raw_offset, uint16_t len, item_type type, uint8_t flags = 0)
            : raw_data_offset(raw_offset), data_length(len), type(type), flags(flags) {}
    };

    /**
     * @struct repetitive_item
     * @brief Represents a repetitive item. It is immediately followed in memory by `child_count` uint32_t offsets.
     */
    struct repetitive_item
    {
        item        header;
        uint16_t    child_count;

        inline repetitive_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, item_type_repetetive), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(repetitive_item));
        }
        
        inline const uint32_t* get_child_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(repetitive_item));
        }
    };

    /**
     * @struct compound_item
     * @brief Represents a compound item. It is immediately followed in memory by `child_count` uint32_t offsets.
     */
    struct compound_item
    {
        item        header;
        uint16_t    child_count; // Number of items actually present based on FSPEC

        inline compound_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, item_type_compound), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(compound_item));
        }
        
        inline const uint32_t* get_child_offsets() const 
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(compound_item));
        }
    };

    /**
     * @struct explicit_item
     * @brief Represents an explicit item, which might have its own children (recursive).
     */
    struct explicit_item
    {
        item        header;
        uint16_t    child_count;

        inline explicit_item(uint32_t raw_offset, uint16_t len, uint16_t child_count)
            : header(raw_offset, len, item_type_explicit), child_count(child_count) {}

        // Following memory contains: uint32_t child_offsets[child_count];
        inline uint32_t* get_child_offsets() 
        {
            return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(this) + sizeof(explicit_item));
        }
        
        inline const uint32_t* get_child_offsets() const
        {
            return reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(explicit_item));
        }
    };

    #pragma pack(pop)
}
