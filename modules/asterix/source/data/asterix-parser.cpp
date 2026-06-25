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

    static inline bool critical_parser_error(adam::buffer*& internal_data)
    {
        adam::buffer_manager::get().return_buffer(internal_data);
        internal_data = nullptr;
        return false;
    }

    static inline bool resize_internal_data(uint32_t new_size_hint, adam::buffer*& internal_data)
    {
        auto newcap = internal_data->get_capacity();

        while (newcap < new_size_hint)
            newcap = static_cast<uint32_t>(newcap * 1.5f);
        
        auto* newbuff = adam::buffer_manager::get().request_buffer(newcap);

        if (!newbuff)
            return false;

        std::memcpy(newbuff->data(), internal_data->begin(), internal_data->get_size());
        newbuff->set_size(internal_data->get_size());

        auto* oldbuf = internal_data;
        internal_data = newbuff;

        adam::buffer_manager::get().return_buffer(oldbuf);

        return true;
    }

    static inline bool reserve_internal_data(uint32_t size_to_reserve, uint32_t& out_offset, adam::buffer*& internal_data)
    {
        if (internal_data->get_remaining_capacity() < size_to_reserve)
        {
            if (!resize_internal_data(internal_data->get_size() + size_to_reserve, internal_data))
                return false;
        }

        out_offset = internal_data->get_size();
        internal_data->set_size(out_offset + size_to_reserve);

        return true;
    }

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
        uint8_t fspec_octets = (sub_uap->get_highest_frn() + 7) / 8; // Bytes needed to represent all FRN bits
        if (raw_offset + fspec_octets > raw_length) return false;

        for (uint8_t oct = 0; oct < fspec_octets; ++oct)
        {
            uint8_t val = raw_data[raw_offset + oct];
            for (int bit = 7; bit >= 0; --bit)  // All 8 bits are data bits — no FX
            {
                uint16_t frn = static_cast<uint16_t>((oct * 8) + (8 - bit));

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

        auto itm = internal_data->at<item>(out_offset);

        itm->type           = item_type_fixed;
        itm->raw_offset     = raw_offset;
        itm->raw_length     = spec.data_size;

        out_offset += sizeof(item);
        raw_offset += spec.data_size;

        return true;
    }

    static inline bool parse_variable_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t raw_start_offset   = raw_offset;
        uint16_t first_size         = spec.header_size;
        uint16_t extent_size        = spec.data_size;

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
        uint16_t total_len = static_cast<uint16_t>(raw_offset - raw_start_offset);

        auto itm = internal_data->at<item>(out_offset);

        itm->type           = item_type_variable;
        itm->raw_offset     = raw_start_offset;
        itm->raw_length     = total_len;

        out_offset += sizeof(item);

        return true;
    }

    static inline bool parse_repetitive_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t raw_start_offset  = raw_offset;
        uint8_t  ind_len           = spec.header_size;

        // Check raw buffer boundary for reading the repetition count
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

        uint32_t rep_size = static_cast<uint32_t>(rep) * spec.data_size;

        // Check raw buffer boundary for reading all repetitions
        if (raw_offset + rep_size > raw_length) return false;

        raw_offset += rep_size;
        
        auto itm = internal_data->at<item>(out_offset);

        itm->type           = item_type_repetetive;
        itm->raw_offset     = raw_start_offset;
        itm->raw_length     = static_cast<uint16_t>(raw_offset - raw_start_offset);

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
        uint32_t&           child_out_offset
    )
    {
        uint32_t start_out_off  = out_offset;
        uint32_t items_size     = cur_uap->get_highest_frn() * sizeof(item);

        // Reserve Space for items
        out_offset += items_size;

        // Zero out all items
        std::memset(internal_data->at<item>(start_out_off), 0, items_size); // I tested only setting the populated bit, but the memset was actually faster. So yea zero everything

        for (uint8_t i = 1; i <= cur_uap->get_highest_frn(); ++i)
        {
            if (!active_child_frns[i])
                continue;

            uint32_t cur_item_offset = start_out_off + ((i-1) * sizeof(item));

            const field_spec* child_spec = cur_uap->get_spec(i);

            // Current UAP may not have a spec for frn i
            if (!child_spec)
            {
                // Log - Missing item spec
                continue;
            }

            auto my_child_out_off   = child_out_offset;
            auto parse_item_off     = cur_item_offset;

            if (!parse_item(*child_spec, raw_data, raw_offset, raw_length, internal_data, parse_item_off, my_child_out_off))
            {
                // TODO do we return here? just output a log? IDK
                return false;
            }

            // Calculate added child items - Using static UAP spec info instead of dynamic offsets
            uint16_t cur_child_count = 0;
            if ((child_spec->type == item_type_compound || child_spec->type == item_type_explicit) && child_spec->sub_uap)
                cur_child_count = child_spec->sub_uap->get_highest_frn();

            auto* cur_item = internal_data->at<item>(cur_item_offset);
            cur_item->set_populated();
            cur_item->child_offset = cur_child_count ? (child_out_offset - cur_item_offset) : 0; // Until now, we used a absolute child offset, but we want it to be relative to the current item, so we convert it here
            cur_item->child_count  = cur_child_count;

            child_out_offset = my_child_out_off;
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

        // Save current offsets
        uint32_t item_start_offset      = out_offset;
        uint32_t raw_item_start_offset  = raw_offset;

        bool active_frns[asterix::highest_frn];
        std::memset(active_frns, 0, spec.sub_uap->get_highest_frn() + 1);

        if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns)) return false;
        
        // Reserve Space for item + child items
        out_offset += sizeof(item);
        uint32_t child_items_offset = child_offset + spec.sub_uap->get_highest_frn() * sizeof(item);

        if (!parse_children
        (
            spec.sub_uap, 
            raw_data, 
            raw_offset, 
            raw_length, 
            active_frns, 
            internal_data,
            child_offset,
            child_items_offset
        ))
        {
            return false;
            // TODO what now? log
        }
        child_offset = child_items_offset;

        auto* itm = internal_data->at<item>(item_start_offset);
        itm->type           = item_type_compound;
        itm->raw_offset     = raw_item_start_offset;
        itm->raw_length     = static_cast<uint16_t>(raw_offset - raw_item_start_offset);

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
        uint32_t item_start_offset      = out_offset;
        uint32_t raw_item_start_offset  = raw_offset;

        if (raw_offset >= raw_length) return false;

        uint8_t explicit_len = raw_data[raw_offset]; // length includes this byte
        if (raw_offset + explicit_len > raw_length) return false;

        // Reserve Space for item
        out_offset += sizeof(item);

        auto* itm = internal_data->at<item>(item_start_offset);
        itm->type           = item_type_explicit;
        itm->raw_offset     = raw_item_start_offset;
        itm->raw_length     = explicit_len;

        if (spec.sub_uap)
        {
            raw_offset += 1; // skip length byte before FSPEC
            
            bool active_frns[asterix::highest_frn];
            std::memset(active_frns, 0, spec.sub_uap->get_highest_frn() + 1);

            if (!parse_explicit_fspec(spec.sub_uap, raw_data, raw_offset, raw_length, active_frns)) return false;

            // Reserve Space for child items
            uint32_t child_items_offset = child_offset + spec.sub_uap->get_highest_frn() * sizeof(item);

            if (!parse_children
            (
                spec.sub_uap, 
                raw_data, 
                raw_offset, 
                raw_length, 
                active_frns, 
                internal_data,
                child_offset,
                child_items_offset
            ))
            {
                return false;
                // TODO what now? log
            }
            child_offset = child_items_offset;
        }
        else
        {
            // Unparsed explicit payload — no manual parsing possible
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

        // Pre-pass to estimate required capacity based on actual block layouts and record density.
        // This avoids allocating massive buffers (e.g. 26MB for 64KB inputs) which thrash L1/L2 caches and bypass thread-local caches.
        uint32_t required_capacity = sizeof(frame);
        uint32_t scan_offset = 0;
        while (scan_offset < raw_length)
        {
            if (scan_offset + sizeof(raw_block_header) > raw_length)
                break;

            const auto* raw_block_head = reinterpret_cast<const raw_block_header*>(raw_data + scan_offset);
            uint16_t block_len = raw_block_head->get_length();
            if (block_len < min_block_length || scan_offset + block_len > raw_length)
                break;

            uint8_t category = raw_block_head->category;
            const uap* active_uap = uap_pool::get().get_uap(category);
            uint32_t max_frn = active_uap ? active_uap->get_highest_frn() : 40;

            // Safe educated guess: assume an average minimum record size of 6 bytes (including FSPEC and data).
            // A record containing data (e.g., DSID + TOD) can almost never be smaller than 6 bytes.
            uint32_t payload_len = (block_len > sizeof(raw_block_header)) ? (block_len - sizeof(raw_block_header)) : 0;
            uint32_t est_records = std::max(1u, payload_len / asterix::min_block_length + 1);

            // Reserve space for the record headers, direct UAP items, and 10 extra items per record for compound/explicit children.
            uint32_t est_items = est_records * (max_frn + 10);

            required_capacity += sizeof(block) + est_records * sizeof(record) + est_items * sizeof(item);
            scan_offset += block_len;
        }

        // Safe fallback in case of parsing errors or empty/malformed packets in scan
        if (required_capacity <= sizeof(frame))
        {
            required_capacity = sizeof(frame) + sizeof(block) + sizeof(record) + 40 * sizeof(item);
        }

        internal_data = adam::buffer_manager::get().request_buffer(required_capacity);
        if (!internal_data) return false;

        internal_data->set_referenced_buffer(buf);

        uint32_t raw_offset = 0;
        uint32_t out_offset = 0;

        // Reserve space for frame
        uint32_t frame_off = out_offset;
        out_offset += sizeof(frame);

        uint16_t block_count = 0;
        uint32_t last_block_offset = 0;

        while (raw_offset < raw_length)
        {
            // Block begin
            uint32_t block_start_raw    = raw_offset;
            const auto* raw_block_head  = reinterpret_cast<const raw_block_header*>(raw_data + block_start_raw);

            if (raw_offset + sizeof(raw_block_header) > raw_length) 
                return critical_parser_error(internal_data);

            uint16_t block_len = raw_block_head->get_length();
            if (block_len < min_block_length || block_start_raw + block_len > raw_length) 
                return critical_parser_error(internal_data);

            raw_offset += sizeof(raw_block_header);

            uint32_t block_end_raw = block_start_raw + block_len;

            // At this point we have a valid block
            block_count++;

            // Reserve space for block
            uint32_t block_offset = out_offset;
            out_offset += sizeof(block);

            if (last_block_offset)
            {
                auto* last_block = internal_data->at<block>(last_block_offset);
                last_block->set_has_next();
            }

            last_block_offset = block_offset;

            uint16_t record_count       = 0;
            uint16_t record_items_count = 0;
            uint32_t last_record_offset = 0;

            // Parse records in the block
            while (raw_offset < block_end_raw)
            {
                bool active_frns[asterix::highest_frn];

                uint32_t record_start_raw = raw_offset;
                const auto* raw_rec_head = reinterpret_cast<const raw_record_header*>(raw_data + record_start_raw);
            
                if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns))
                {
                    // Log FSPEC invalid, move to next block
                    raw_offset = block_end_raw; break;
                }

                uint8_t fspec_size = static_cast<uint8_t>(raw_offset - record_start_raw);
                auto resolved_frns = (fspec_size * 7);

                // Retrieve the correct uap for the given record, this will automatically handle alterantive uaps
                const uap* active_uap = raw_rec_head->retrieve_uap(raw_block_head, fspec_size, raw_length);
                if (!active_uap)
                {
                    // Log UAP not available, move to next block
                    raw_offset = block_end_raw; break;
                }
                
                // Set possible remaining active_frns to false
                if (resolved_frns < active_uap->get_highest_frn())
                    std::memset(active_frns + resolved_frns + 1, 0, active_uap->get_highest_frn() - resolved_frns);

                if (internal_data->get_capacity() < out_offset + sizeof(record))
                {
                    // Out buffer too small for additional record, move to next block
                    raw_offset = block_end_raw; break;
                }

                // At this point we have a valid record
                record_count++;

                // Reserve space for record
                uint32_t record_offset = out_offset;
                out_offset += sizeof(record);

                if (last_record_offset)
                {
                    auto* last_record = internal_data->at<record>(last_record_offset);
                    last_record->set_has_next();
                }

                last_record_offset = record_offset;

                // Save the offset where the child items for the current record will be located
                uint32_t child_items_offset = out_offset + active_uap->get_highest_frn() * sizeof(item);
                uint32_t child_offset       = child_items_offset;

                if (!parse_children
                (
                    active_uap,
                    raw_data,
                    raw_offset,
                    raw_length,
                    active_frns,
                    internal_data,
                    out_offset,
                    child_offset
                ))
                {
                    break;
                    // TODO log, and then?
                }

                out_offset += child_offset - child_items_offset; // Move out_offset to the end of the child items

                auto child_count = static_cast<uint16_t>((child_offset - child_items_offset) / sizeof(item));

                auto* cur_rec = internal_data->at<record>(record_offset);
                cur_rec->category   = active_uap->get_cat_number();
                cur_rec->used_uap   = active_uap->get_name().get_hash();
                cur_rec->fspec_size = fspec_size;
                cur_rec->raw_length = static_cast<uint16_t>(raw_offset - record_start_raw);
                cur_rec->raw_offset = record_start_raw;
                cur_rec->item_count = active_uap->get_highest_frn() + child_count; // Total items = direct items + child items
                cur_rec->flags      = record_flag_none;

                record_items_count += cur_rec->item_count;
            }
       
            auto* cur_blk = internal_data->at<block>(block_offset);
            cur_blk->category     = raw_block_head->category;
            cur_blk->raw_length   = block_len;
            cur_blk->raw_offset   = block_start_raw;
            cur_blk->record_count = record_count;
            cur_blk->item_count   = record_items_count;
            cur_blk->flags        = block_flag_none;
        }

        auto* cur_frame = internal_data->at<frame>(frame_off);
        cur_frame->block_count = block_count;
        cur_frame->flags       = frame_flag_none;
        cur_frame->reserved    = 0;
        
        internal_data->set_size(out_offset);

        return true;
    }

}