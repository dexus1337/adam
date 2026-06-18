#include "data/asterix-parser.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    //Foward declare
    static inline bool parse_item
    (
        const field_spec&   spec, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        adam::buffer*&      internal_data, 
        uint32_t&           out_offset, 
        uint32_t&           child_offset
    );

    static inline bool parse_fspec
    (
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        bool*               active_frns
    )
    {
        if (raw_offset >= raw_length) return false;

        const auto* fspec = reinterpret_cast<const raw_fspec*>(raw_data + raw_offset);
        uint8_t oct = 0;

        while (fspec)
        {
            uint32_t current_offset = raw_offset + oct;
            if (current_offset >= raw_length) return false;

            for (int bit = 7; bit >= 1; --bit)
            {
                uint16_t frn = static_cast<uint16_t>((oct * 7) + (8 - bit));

                if (frn >= asterix::highest_frn) return false;

                active_frns[frn] = ((fspec->value >> bit) & 1);
            }
            fspec = fspec->get_next();
            oct++;
        }

        raw_offset += oct;
        return true;
    }

    static inline bool parse_explicit_fspec
    (
        const uap*          sub_uap, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        bool*               active_frns
    )
    {
        uint8_t fspec_octets = (sub_uap->last_frn + 7) / 8; // Bytes needed to represent all FRN bits
        if (raw_offset + fspec_octets > raw_length) return false;

        for (uint8_t oct = 0; oct < fspec_octets; ++oct)
        {
            uint8_t val = raw_data[raw_offset + oct];
            for (int bit = 7; bit >= 0; --bit)  // All 8 bits are data bits — no FX
            {
                uint16_t frn = static_cast<uint16_t>((oct * 7) + (8 - bit));

                if (frn >= asterix::highest_frn) return false;

                active_frns[frn] = ((val >> bit) & 1);
            }
        }
        raw_offset += fspec_octets;
        return true;
    }

    static inline bool parse_fixed_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        // Raw data unused here
        (void)raw_data;

        // Check raw buffer boundary
        if (raw_offset + spec.data_size > raw_length) return false;

        auto test = internal_data->at<item>(out_offset);

        new(test) item(item_type_fixed, spec.data_size, raw_offset);
        out_offset += sizeof(item);

        raw_offset += spec.data_size;

        return true;
    }

    static inline bool parse_variable_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t item_start_offset = raw_offset;
        uint16_t first_size = spec.header_size;
        uint16_t extent_size = spec.data_size;

        // Check raw buffer boundary
        if (raw_offset + first_size > raw_length) return false;

        raw_offset += first_size;
        bool fx = (raw_data[raw_offset - 1] & 1) != 0;

        while (fx)
        {
            // Check raw buffer boundary
            if (raw_offset + extent_size > raw_length) return false;

            raw_offset += extent_size;
            fx = (raw_data[raw_offset - 1] & 1) != 0;
        }
        uint16_t total_len = static_cast<uint16_t>(raw_offset - item_start_offset);

        new(internal_data->at<item>(out_offset)) item(item_type_variable, total_len, item_start_offset);
        out_offset += sizeof(item);

        return true;
    }

    static inline bool parse_repetitive_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t item_start_offset = raw_offset;
        uint8_t ind_len = spec.header_size;

        // Check raw buffer boundary
        if (ind_len == 0 || raw_offset + ind_len > raw_length) return false;

        uint16_t rep = 0;

        switch (ind_len)
        {
        case 1:
            rep = raw_data[raw_offset];
            break;
        case 2:
            rep = adam::swap_2(raw_data + raw_offset);
            break;
        
        default:
            return false;
        }
        
        raw_offset += ind_len;

        new(internal_data->at<item>(out_offset)) item(item_type_repetetive, rep, item_start_offset);
        out_offset += sizeof(item);

        return true;
    }

    static inline bool parse_children
    (
        const uap*          cur_uap,
        const uint8_t*      raw_data,
        uint32_t&           raw_offset,
        uint32_t            raw_length,
        const bool*         active_child_frns,
        adam::buffer*&      internal_data,
        uint32_t&           out_offset,
        uint32_t&           child_out_offset,
        uint16_t&           child_count
    )
    {
        uint32_t start_out_off = out_offset;

        // Reserve Items for items
        out_offset += cur_uap->last_frn * sizeof(item);

        for (uint8_t i = 1; i < cur_uap->last_frn; ++i)
        {
            uint32_t cur_item_offset = start_out_off + ((i-1) * sizeof(item));

            internal_data->at<item>(cur_item_offset)->populated = active_child_frns[i];

            if (!active_child_frns[i])
                continue;

            const field_spec* child_spec = cur_uap->get_spec(i);

            // Current UAP may not have a spec for frn i
            if (!child_spec)
            {
                // Log - Missing item spec
                internal_data->at<item>(cur_item_offset)->populated = false;
                continue;
            }

            auto my_child_out_off = child_out_offset;

            if (!parse_item(*child_spec, raw_data, raw_offset, raw_length, internal_data, cur_item_offset, my_child_out_off))
            {
                // TODO do we return here? just output a log? IDK
                continue;
            }

            // Calculate added child items - Could've just passed a child count param, but parse_item already has enough params
            auto cur_child_count = static_cast<uint16_t>((my_child_out_off - child_out_offset) / sizeof(item));

            internal_data->at<item>(cur_item_offset)->child_offset = cur_child_count ? child_out_offset : 0;
            internal_data->at<item>(cur_item_offset)->child_count  = cur_child_count;

            child_count += cur_child_count;
        }

        return true;
    }

    static inline bool parse_compound_item
    (
        const field_spec&   spec, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        adam::buffer*&      internal_data, 
        uint32_t&           out_offset, 
        uint32_t&           child_offset
    )
    {
        // Compound must have sub_uap, no length field whatsoever
        if (!spec.sub_uap) return false; 

        uint32_t raw_item_start_offset = raw_offset;

        bool active_frns[asterix::highest_frn];
        std::memset(active_frns, 0, spec.sub_uap->last_frn);

        if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns)) return false;

        new(internal_data->at<item>(out_offset)) item(item_type_compound, 0, raw_item_start_offset);
        out_offset += sizeof(item);

        // Save the offset where the child items for the current item will be located
        uint32_t child_items_offset = child_offset + spec.sub_uap->last_frn * sizeof(item);

        uint16_t child_count = 0;

        if (!parse_children
        (
            spec.sub_uap, 
            raw_data, 
            raw_offset, 
            raw_length, 
            active_frns, 
            internal_data,
            child_offset,
            child_items_offset,
            child_count
        ))
        {
            // TODO what now? log
        }

        auto* itm = internal_data->at<item>(raw_item_start_offset);
        itm->raw_length     = raw_offset - raw_item_start_offset;
        itm->child_count    = spec.sub_uap->last_frn;
        itm->child_offset   = child_offset;

        return true;
    }

    static inline bool parse_explicit_item
    (
        const field_spec&   spec, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        adam::buffer*&      internal_data, 
        uint32_t&           out_offset, 
        uint32_t&           child_offset
    )
    {
        uint32_t raw_item_start_offset = raw_offset;

        if (raw_offset >= raw_length) return false;

        uint8_t explicit_len = raw_data[raw_offset]; // length includes this byte
        if (raw_offset + explicit_len > raw_length) return false;

        new(internal_data->at<item>(out_offset)) item(item_type_compound, explicit_len, raw_item_start_offset);
        out_offset += sizeof(item);

        if (spec.sub_uap)
        {
            raw_offset += 1; // skip length byte before FSPEC
            
            bool active_frns[asterix::highest_frn];
            std::memset(active_frns, 0, spec.sub_uap->last_frn);
            
            if (!parse_explicit_fspec(spec.sub_uap, raw_data, raw_offset, explicit_len, active_frns)) return false;

            // Save the offset where the child items for the current item will be located
            uint32_t child_items_offset = child_offset + spec.sub_uap->last_frn * sizeof(item);
            uint16_t child_count = 0;

            if (!parse_children
            (
                spec.sub_uap, 
                raw_data, 
                raw_offset, 
                raw_length, 
                active_frns, 
                internal_data,
                child_offset,
                child_items_offset,
                child_count
            ))
            {
                // TODO what now? log
            }

            auto* itm = internal_data->at<item>(raw_item_start_offset);
            itm->raw_length     = raw_offset - raw_item_start_offset;
            itm->child_count    = child_count;
            itm->child_offset   = child_items_offset;
        }
        else
        {
            // Unparsed explicit payload — skip the whole thing
            raw_offset += explicit_len;
        }
        return true;
    }

    static inline bool parse_item
    (
        const field_spec&   spec, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        adam::buffer*&      internal_data, 
        uint32_t&           out_offset, 
        uint32_t&           child_offset
    )
    {
        switch (spec.type)
        {
            case item_type_fixed:
                return parse_fixed_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset);
            case item_type_variable:
                return parse_variable_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset);
            case item_type_repetetive:
                return parse_repetitive_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset);
            case item_type_compound:
                return parse_compound_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset, child_offset);
            case item_type_explicit:
                return parse_explicit_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset, child_offset);
            default:
                return false;
        }
    }

    bool asterix_parser::parse(class adam::buffer* buf, class adam::buffer*& internal_data)
    {
        if (!buf) return false;

        // First of all, we expect only whole blocks in here, so make sure to use a destreamer ahead of this parser
        uint32_t raw_length = buf->get_size();
        const uint8_t* raw_data = buf->get_begin_as<uint8_t>();

        // To get a (hopefully) fitting buffer first try, expect the "worst" aka. highest amount of blocks possible
        uint32_t max_blocks         = raw_length / asterix::minimum_block_length;
        uint32_t required_capacity  = max_blocks * sizeof(item) * 100; // Wild "high" guess to have an higest average of 100 items per record

        internal_data = adam::buffer_manager::get().request_buffer(required_capacity);
        if (!internal_data) return false;

        internal_data->set_referenced_buffer(buf);

        uint32_t raw_offset = 0;
        uint32_t out_offset = 0;

        // Reserve space for frame
        uint32_t frame_off = out_offset;
        out_offset += sizeof(frame);

        uint16_t block_count = 0;

        while (raw_offset < raw_length)
        {
            uint32_t block_start_raw = raw_offset;

            if (raw_offset + sizeof(raw_block_header) > raw_length)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                // Log frame too small
                return false;
            }

            const auto* raw_header = reinterpret_cast<const raw_block_header*>(raw_data + raw_offset);
            const uap* active_uap = raw_header->get_uap();
            if (!active_uap)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                // Log UAP not available
                return false;
            }

            uint16_t block_len = raw_header->get_length();
            if (block_len < minimum_block_length || block_start_raw + block_len > raw_length)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                // Log Block too small or block too big -> size mismatch
                return false;
            }

            raw_offset += sizeof(raw_block_header);

            uint32_t block_end_raw = block_start_raw + block_len;

            // At this point we have a valid block
            block_count++;

            // Reserve space for block
            uint32_t block_off = out_offset;
            out_offset += sizeof(block);

            uint16_t record_count       = 0;
            uint16_t record_items_count = 0;

            // Parse records in the block
            while (raw_offset < block_end_raw)
            {
                bool active_frns[asterix::highest_frn];
                std::memset(active_frns, 0, active_uap->last_frn);

                uint32_t record_start_raw = raw_offset;

                if (!parse_fspec(raw_data, raw_offset, block_len, active_frns))
                {
                    // TODO: FSpec invalid, what now? discard everything? probably only current parsed record
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                if (internal_data->get_capacity() < out_offset + sizeof(record))
                {
                    // Out buffer too small for additional record, TODO: resize
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                // At this point we have a valid record
                record_count++;

                // Reserve space for record
                uint32_t record_off = out_offset;
                out_offset += sizeof(record);

                // Save the offset where the child items for the current record will be located
                uint32_t child_items_offset = out_offset + active_uap->last_frn * sizeof(item);

                uint16_t child_count = 0;

                if (!parse_children
                (
                    active_uap,
                    raw_data,
                    raw_offset,
                    block_len,
                    active_frns,
                    internal_data,
                    out_offset,
                    child_items_offset,
                    child_count
                ))
                {
                    // TODO log, and then?
                }

                auto* cur_rec = internal_data->at<record>(record_off);
                cur_rec->category   = active_uap->cat_id;
                cur_rec->raw_length = raw_offset - record_start_raw;
                cur_rec->raw_offset = record_start_raw;
                cur_rec->item_count = active_uap->last_frn + child_count;

                record_items_count += cur_rec->item_count;
            }
       
            auto* cur_blk = internal_data->at<block>(block_off);
            cur_blk->category     = active_uap->cat_id;
            cur_blk->raw_length   = block_len;
            cur_blk->raw_offset   = block_start_raw;
            cur_blk->record_count = record_count;
            cur_blk->item_count   = record_items_count;
        }

        internal_data->at<frame>(frame_off)->block_count = block_count;
        
        internal_data->set_size(out_offset);

        return true;
    }
}