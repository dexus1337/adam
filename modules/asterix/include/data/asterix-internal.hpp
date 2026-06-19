#pragma once

/**
 * @file    asterix-internal.hpp
 * @author  dexus1337
 * @brief   Defines the internal parsed zero-copy tree nodes and headers for Asterix.
 * 
 * This is the memory layout of a parsed frame
 * 
 *  ----------------------------
 * |          FRAME            |
 * | - block_count             |
 * ----------------------------
 * 
 *  ----------------------------
 * |          BLOCK            |
 * | - record_count            |
 * ----------------------------
 * 
 *  ----------------------------
 * |          RECORD           |
 * | - item_count              |
 * ----------------------------
 * 
 *  ----------------------------    <- An populated item
 * |     ITEM FRN 1 (fixed)    |
 * | - child_count = 0         |
 * | - populated = true        |
 * ----------------------------
 *  ----------------------------    <- An NOT populated item, this ensures fast O(1) access for any FRN
 * |     ITEM FRN 2 (fixed)    |
 * | - populated = false       |
 * ----------------------------
 * ...
 *  ----------------------------    <- An item WITH childs. The childs are populated
 * |  ITEM FRN 15 (explicit)   |
 * | - child_count = 1         |
 * | - populated = true        |
 * | - child_offset = 0x180    |    <- Beginning of Child Item "Array" is saved in the parent (absolute offset from internal_data start)
 * ----------------------------
 * ...
 *  ----------------------------    <- Every possible Item for an UAP is present in memory
 * |  ITEM FRN LAST            |
 * | - populated = false       |
 * ----------------------------
 * 
 * @0x180 (First Child)
 *  ----------------------------    <- An populated item CHILD item
 * |     ITEM FRN 1 (fixed)    |
 * | - child_count = 0         |
 * | - populated = true        |
 * ----------------------------
 * ...
 *  ----------------------------    <- Every possible Item for an UAP is present in memory, for Children, too
 * |  ITEM FRN LAST            |
 * | - populated = false       |
 * ----------------------------
 * 
 * @version 1.0
 * @date    16.06.2026
 */

#include "api/api-asterix.hpp"
#include <cstdint>
#include <types/byteswap.hpp>

namespace adam::modules::asterix
{
    class uap;

    /**
     * @enum    item_type
     * @brief   The type of an asterix item.
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
     * @struct  item
     * @brief   The common header for all parsed asterix items within the buffer.
     */
    struct item
    {
        uint16_t    child_count;    /**< Number of Children at "child_offset". */
        uint16_t    raw_length;     /**< Length of the data in bytes. */
        uint32_t    raw_offset;     /**< Offset to the raw byte data in the referenced source buffer. */
        uint32_t    child_offset;   /**< Offset to the child "items"'s from the current object pointer ("this"). */
        item_type   type;           /**< Type of the item. */
        bool        populated;      /**< Marks the item as existing, can be set to false to remove item */

        item(){}
        item(item_type type, uint16_t raw_len, uint32_t raw_off) 
         :  child_count(0),
            raw_length(raw_len), 
            raw_offset(raw_off), 
            type(type),
            populated(true) {}

        inline const item* get_child_item(uint8_t frn) const 
        { 
            if (frn > child_count) return nullptr;
            return reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + child_offset) + (frn - 1); 
        }
    };

    /**
     * @struct  record
     * @brief   Repopulateds a parsed Asterix record within a block.
     */
    struct record
    {
        uint16_t item_count;    /**< Number of items in this record. */
        uint16_t raw_length;    /**< Length of raw data for this record. */
        uint32_t raw_offset;    /**< Offset in raw buffer where this record starts. */
        uint8_t  category;      /**< Asterix category (e.g. 48, 62). */
        uint8_t  fspec_size;    /**< Size of the FSPEC in bytes */
        
        record(uint8_t category, uint8_t fspec_size, uint16_t items, uint16_t raw_len, uint32_t raw_off) : 
            item_count(items), 
            raw_length(raw_len), 
            raw_offset(raw_off),
            category(category),
            fspec_size(fspec_size) {}

        inline const uap* get_uap() const; // defined in asterix-uap.hpp
        
        inline const item* get_item(uint8_t frn) const 
        { 
            if (!frn || frn > item_count) return nullptr; 
            
            return reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record)) + (frn - 1); 
        }
    };

    /**
     * @struct  block
     * @brief   Parsed block data. Keeps track of number of record and total number of items in there
     */
    struct block
    {
        uint16_t record_count;  /**< Number of records in this block. */
        uint16_t raw_length;    /**< Length of raw data for this block. */
        uint32_t raw_offset;    /**< Offset in raw buffer where this block starts. */
        uint16_t item_count;    /**< Total number of items (of all records) in this block. */
        uint8_t  category;      /**< Asterix category (e.g. 48, 62). */

        block(uint8_t category, uint16_t record_count, uint16_t raw_len, uint32_t raw_off) 
         :  record_count(record_count), 
            raw_length(raw_len),
            raw_offset(raw_off),
            category(category) {}

        inline const uap* get_uap() const; // defined in asterix-uap.hpp

        /** @brief Record getter. CAUTION: O(n) Access, slow. Use iterator for iterating */
        inline const record* get_record(uint16_t idx) const 
        {
            if (idx >= record_count) return nullptr;
            auto recstart = reinterpret_cast<const uint8_t*>(this) + sizeof(block);
            for (size_t i = 0; i < idx; i++)
                recstart += reinterpret_cast<const record*>(recstart)->item_count * sizeof(item) + sizeof(record);
            return reinterpret_cast<const record*>(recstart);
        }
    };

    /**
     * @struct  frame
     * @brief   Header at the very start of the parsed internal_data buffer.
     */
    struct frame
    {
        uint16_t block_count;

        frame(uint16_t blocks) : block_count(blocks) {}

        /** @brief Record getter. CAUTION: O(n) Access, slow. Use iterator for iterating */
        inline const block* get_block(uint16_t idx) const 
        {
            if (idx >= block_count) return nullptr;
            auto itr = reinterpret_cast<const uint8_t*>(this) + sizeof(frame);
            for (size_t i = 0; i < idx; i++)
            {
                auto blk = reinterpret_cast<const block*>(itr);
                itr += sizeof(block);
                itr += blk->record_count * sizeof(record);
                itr += blk->item_count * sizeof(item);
            }
            return reinterpret_cast<const block*>(itr);
        }
    };

    #pragma pack(pop)
}
