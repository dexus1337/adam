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
#include "api/api-sdk.hpp"
#include <adam-sdk.hpp>
#include <cstdint>
#include <iterator>
#include <types/byteswap.hpp>

namespace adam::modules::asterix
{
    class uap;

    struct item;
    struct record;
    struct block;
    struct frame;

    /**
     * @struct  internal_type_iterator
     * @brief   Forward iterator over the variable-length asterix internal format types (blocks & records)
     */
    template<typename internal_type>
    struct internal_type_iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = const internal_type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const internal_type*;
        using reference         = const internal_type&;

        explicit internal_type_iterator(pointer ptr) : m_ptr(ptr) {}

        inline reference operator*()    const { return *m_ptr; }
        inline pointer operator->()     const { return  m_ptr; }

        inline internal_type_iterator& operator++() 
        {
            if (m_ptr)
            {
                m_ptr = m_ptr->get_next();
            }
            return *this;
        }

        inline internal_type_iterator operator++(int)
        {
            internal_type_iterator temp = *this;
            ++(*this); // calls the upper ++ opeator on the local copy reference
                return temp;
        }
        
        inline bool operator==(const internal_type_iterator& rhs) const { return m_ptr == rhs.m_ptr; }
        inline bool operator!=(const internal_type_iterator& rhs) const { return m_ptr != rhs.m_ptr; }

    private:
        pointer m_ptr;
    };

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

    enum item_flag : uint8_t
    {
        item_flag_none      = 0,
        item_flag_populated = 1 << 0,
        item_flag_child     = 1 << 1,
        item_flag_modified  = 1 << 2
    };
    enable_enum_bit_operations(item_flag);

    enum record_flag : uint8_t
    {
        record_flag_none     = 0,
        record_flag_modified = 1 << 0,
        record_flag_has_next = 1 << 1
    };
    enable_enum_bit_operations(record_flag);

    enum block_flag : uint8_t
    {
        block_flag_none     = 0,
        block_flag_modified = 1 << 0,
        block_flag_has_next = 1 << 1
    };
    enable_enum_bit_operations(block_flag);

    enum frame_flag : uint8_t
    {
        frame_flag_none     = 0,
        frame_flag_modified = 1 << 0
    };
    enable_enum_bit_operations(frame_flag);

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
        item_flag   flags;          /**< Flags for the item. e.g. populated, modified. */

        inline const item* get_child_item(uint8_t frn) const 
        { 
            if (frn > child_count) return nullptr;
            return reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + child_offset) + (frn - 1); 
        }

        inline bool is_populated()  const { return flags & item_flag_populated; }
        inline bool is_child()      const { return flags & item_flag_child;     }
        inline bool is_modified()   const { return flags & item_flag_modified;  }
        
        inline void set_populated(bool populated = true) 
        {
            if (populated)
                flags |= item_flag_populated;
            else
                flags &= ~item_flag_populated;
        }

        inline void set_modified(bool modified = true)
        {
            if (modified)
                flags |= item_flag_modified;
            else
                flags &= ~item_flag_modified;
        }
    };

    /**
     * @struct  record
     * @brief   Repopulateds a parsed Asterix record within a block.
     */
    struct record
    {
        string_hash used_uap;       /**< Hash of the used UAP for this record, used for debugging and verification */
        int32_t     parent_offset;  /**< Offset to the parent record, shall be negative. */
        uint16_t    item_count;     /**< Number of items in this record. Populated + not populated */
        uint16_t    raw_length;     /**< Length of raw data for this record. */
        uint32_t    raw_offset;     /**< Offset in raw buffer where this record starts. */
        uint8_t     category;       /**< Asterix category (e.g. 48, 62). */
        uint8_t     fspec_size;     /**< Size of the FSPEC in bytes */
        record_flag flags;          /**< Flags for the item. e.g. modified. */
        uint8_t     reserved;

        inline const uap*    find_used_uap()    const; // defined in asterix-uap.hpp
        inline const block*  get_parent_block() const { return reinterpret_cast<const block*>(reinterpret_cast<const uint8_t*>(this) + parent_offset); }
        inline const record* get_next()         const { return has_next() ? reinterpret_cast<const record*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record) + item_count * sizeof(item)) : nullptr; }
        
        inline const item* get_item(uint8_t frn) const 
        { 
            if (!frn || frn > item_count) return nullptr; 
            
            return reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record)) + (frn - 1); 
        }

        inline bool is_modified() const { return flags & record_flag_modified; }
        inline void set_modified(bool modified = true)
        {
            if (modified)
                flags |= record_flag_modified;
            else
                flags &= ~record_flag_modified;
        }

        inline bool has_next() const { return flags & record_flag_has_next; }
        inline void set_has_next(bool hash = true)
        {
            if (hash)
                flags |= record_flag_has_next;
            else
                flags &= ~record_flag_has_next;
        }
        
        /**
         * @struct  item_iterator
         * @brief   Forward iterator over the flat item array that follows this record header.
         *
         * Items are stored as a contiguous array of `item` structs immediately after
         * the record header in memory, so this is a plain pointer iterator.
         * Iteration visits ALL slots (including unpopulated ones). Use item::is_populated()
         * to skip gaps, or access by FRN directly via get_item().
         */
        struct item_iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using value_type        = const item;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const item*;
            using reference         = const item&;

            explicit item_iterator(const item* ptr) : m_ptr(ptr) {}

            reference operator*()  const { return *m_ptr; }
            pointer   operator->() const { return  m_ptr; }

            item_iterator& operator++()    { ++m_ptr; return *this; }
            item_iterator  operator++(int) { auto tmp = *this; ++(*this); return tmp; }

            bool operator==(const item_iterator& rhs) const { return m_ptr == rhs.m_ptr; }
            bool operator!=(const item_iterator& rhs) const { return m_ptr != rhs.m_ptr; }

        private:
            const item* m_ptr;
        };

        /** @brief Returns an iterator to the first item slot (FRN 1). */
        inline item_iterator begin() const
        {
            return item_iterator(reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record)));
        }

        /** @brief Returns a past-the-end iterator after the last item slot. */
        inline item_iterator end() const
        {
            return item_iterator(reinterpret_cast<const item*>(reinterpret_cast<const uint8_t*>(this) + sizeof(record)) + item_count);
        }
    };

    /**
     * @struct  block
     * @brief   Parsed block data. Keeps track of number of record and total number of items in there
     */
    struct block
    {
        using record_iterator = internal_type_iterator<record>;
        
        int32_t     parent_offset;  /**< Offset to the parent record, shall be negative. */
        uint16_t    record_count;   /**< Number of records in this block. */
        uint16_t    raw_length;     /**< Length of raw data for this block. */
        uint32_t    raw_offset;     /**< Offset in raw buffer where this block starts. */
        uint16_t    item_count;     /**< Total number of items (of all records) in this block. Populated + not populated. */
        uint8_t     category;       /**< Asterix category (e.g. 48, 62). */
        block_flag  flags;          /**< Flags for the block. e.g. modified. */

        inline const uap*   get_uap()           const; // defined in asterix-uap.hpp
        inline const frame* get_parent_frame()  const { return reinterpret_cast<const frame*>(reinterpret_cast<const uint8_t*>(this) + parent_offset); }
        inline const block* get_next()          const 
        {
            if (!has_next()) return nullptr;
            
            auto ptr = reinterpret_cast<const uint8_t*>(this) + sizeof(block);
            for (uint16_t i = 0; i < record_count; ++i)
            {
                const auto* rec = reinterpret_cast<const record*>(ptr);
                ptr += sizeof(record) + rec->item_count * sizeof(item);
            }
            return reinterpret_cast<const block*>(ptr);
        }
        
        /** @brief Record getter. CAUTION: O(n) Access, slow. Use iterator for iterating */
        inline const record* get_record(uint16_t idx) const 
        {
            if (idx >= record_count) return nullptr;
            auto recstart = reinterpret_cast<const uint8_t*>(this) + sizeof(block);
            for (size_t i = 0; i < idx; i++)
                recstart += reinterpret_cast<const record*>(recstart)->item_count * sizeof(item) + sizeof(record);
            return reinterpret_cast<const record*>(recstart);
        }

        inline bool is_modified() const { return flags & block_flag_modified; }
        inline void set_modified(bool modified = true)
        {
            if (modified)
                flags |= block_flag_modified;
            else
                flags &= ~block_flag_modified;
        }

        inline bool has_next() const { return flags & block_flag_has_next; }
        inline void set_has_next(bool hash = true)
        {
            if (hash)
                flags |= block_flag_has_next;
            else
                flags &= ~block_flag_has_next;
        }

        inline record_iterator begin()  const { return record_iterator(get_record(0)); }
        inline record_iterator end()    const { return record_iterator(nullptr); }
    };

    /**
     * @struct  frame
     * @brief   Header at the very start of the parsed internal_data buffer.
     */
    struct frame
    {
        using block_iterator = internal_type_iterator<block>;
        
        uint32_t    next_offset;
        uint16_t    block_count;
        frame_flag  flags;
        uint8_t     reserved;

        frame(uint16_t blocks) : block_count(blocks), flags(frame_flag_none), reserved(0) {}

        inline const frame* get_next() const { return next_offset ? reinterpret_cast<const frame*>(reinterpret_cast<const uint8_t*>(this) + next_offset) : nullptr; }
        
        /** @brief Block getter. CAUTION: O(n) Access, slow. Use iterator for iterating */
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

        inline bool is_modified() const { return flags & frame_flag_modified; }
        inline void set_modified(bool modified = true)
        {
            if (modified)
                flags |= frame_flag_modified;
            else
                flags &= ~frame_flag_modified;
        }

        inline block_iterator begin()   const { return block_iterator(get_block(0)); }
        inline block_iterator end()     const { return block_iterator(nullptr); }
    };

    #pragma pack(pop)
}
