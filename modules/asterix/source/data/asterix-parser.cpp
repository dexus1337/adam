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
        uint8_t*            active_frns, 
        uint16_t&           active_count
    )
    {
        if (raw_offset >= raw_length) return false;

        const auto* start_fspec = reinterpret_cast<const raw_fspec*>(raw_data + raw_offset);
        active_count = 0;
        uint32_t octet_idx = 0;

        for (auto it = start_fspec->begin(); it != start_fspec->end(); ++it)
        {
            const raw_fspec* current = it.current;
            uint32_t current_offset = raw_offset + octet_idx;
            if (current_offset >= raw_length) return false;

            uint8_t val = current->value;
            for (int bit = 7; bit >= 1; --bit)
            {
                if ((val >> bit) & 1)
                {
                    uint16_t frn = static_cast<uint16_t>((octet_idx * 7) + (8 - bit));
                    if (frn >= asterix::highest_frn) return false; // Bug #3: FRN overflow is malformed data, not a silent skip
                    active_frns[active_count++] = static_cast<uint8_t>(frn);
                }
            }
            octet_idx++;
        }

        raw_offset += octet_idx;
        return true;
    }

    static inline bool parse_explicit_fspec
    (
        const uap*          sub_uap, 
        const uint8_t*      raw_data, 
        uint32_t&           raw_offset, 
        uint32_t            raw_length, 
        uint8_t*            active_frns, 
        uint16_t&           active_count
    )
    {
        uint8_t  highest_frn   = sub_uap->highest_frn;             // Highest registered FRN in this sub-UAP
        uint8_t  fspec_octets  = (highest_frn + 7) / 8;            // Bytes needed to represent all FRN bits
        if (raw_offset + fspec_octets > raw_length) return false;

        active_count = 0;
        for (uint8_t oct = 0; oct < fspec_octets; ++oct)
        {
            uint8_t val = raw_data[raw_offset + oct];
            for (int bit = 7; bit >= 0; --bit)                     // All 8 bits are data bits — no FX
            {
                if ((val >> bit) & 1)
                {
                    uint16_t frn = static_cast<uint16_t>((oct * 8) + (8 - bit));
                    if (frn <= highest_frn)                         // Only accept FRNs within range
                        active_frns[active_count++] = static_cast<uint8_t>(frn);
                }
            }
        }
        raw_offset += fspec_octets;
        return true;
    }

    static inline bool parse_fixed_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        // Raw data unused here
        (void)raw_data;

        uint32_t item_start_offset = raw_offset;

        // Check raw buffer boundary
        if (raw_offset + spec.data_size > raw_length) return false;
        raw_offset += spec.data_size;

        new(internal_data->at<item>(out_offset)) item(item_type_fixed, spec.data_size, item_start_offset);
        out_offset += sizeof(item);

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
        auto* out_ptr = internal_data->begin_as<uint8_t>();
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
        // Compound must have sub_uap
        if (!spec.sub_uap) return false; 

        uint32_t item_start_offset = raw_offset;

        uint8_t active_frns[asterix::highest_frn];
        uint16_t active_count = 0;
        if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns, active_count)) return false;

        
        new(internal_data->at<item>(out_offset)) item(item_type_compound, 0, item_start_offset);
        out_offset += sizeof(item);

        for (uint16_t i = 1; i < spec.sub_uap->highest_frn; ++i)
        {
            // Default set item to not populated
            internal_data->at<item>(child_offset)->populated = false;

            const field_spec* child_spec = spec.sub_uap->get_spec(i);

            // Current UAP may not have a spec for frn i
            if (!child_spec)
                continue;

            if (!parse_item(*child_spec, raw_data, raw_offset, raw_length, internal_data, child_offset, child_offset))
            {
                // TODO do we return here? just output a log? IDK
                continue;
            }
 
            internal_data->at<item>(cur_item_offset)->populated = true;
            
            cur_child_item_offset += sizeof(item);
        }

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
        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();
        if (raw_offset >= raw_length) return false;
        uint8_t explicit_len = raw_data[raw_offset]; // length includes this byte
        if (raw_offset + explicit_len > raw_length) return false;

        if (spec.sub_uap)
        {
            raw_offset += 1; // skip length byte before FSPEC
            uint8_t active_frns[256];
            uint16_t active_count = 0;
            // Bug #1 fix: use fixed-size FSPEC, not FX-extended FSPEC
            if (!parse_explicit_fspec(spec.sub_uap, raw_data, raw_offset, item_start_offset + explicit_len, active_frns, active_count)) return false;

            if (internal_data->get_capacity() < out_offset + sizeof(explicit_item) + (active_count * sizeof(uint32_t))) return false;
            auto* exp_item = new (out_ptr + out_offset) explicit_item(item_start_offset, explicit_len, active_count);
            uint32_t* child_offsets = exp_item->get_child_offsets();
            out_offset += sizeof(explicit_item) + (active_count * sizeof(uint32_t));

            for (uint16_t i = 0; i < active_count; ++i)
            {
                const field_spec* sub_spec = spec.sub_uap->get_spec(active_frns[i]);
                child_offsets[i] = out_offset;
                if (sub_spec)
                {
                    if (!parse_item(*sub_spec, raw_data, raw_offset, item_start_offset + explicit_len, internal_data, out_offset)) return false;
                }
                else
                {
                    return false;
                }
            }

            // Bug #2 fix: pin raw_offset to the declared end of the explicit payload.
            // Sub-items may not fill the entire field (e.g. padding bytes at the end).
            raw_offset = item_start_offset + explicit_len;
        }
        else
        {
            // Unparsed explicit payload — skip the whole thing
            raw_offset += explicit_len;
            if (internal_data->get_capacity() < out_offset + sizeof(explicit_item)) return false;
            new (out_ptr + out_offset) explicit_item(item_start_offset, explicit_len, 0);
            out_offset += sizeof(explicit_item);
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

        uint32_t block_count = 0;

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

            uint32_t record_count = 0;

            // Parse records in the block
            while (raw_offset < block_end_raw)
            {
                uint8_t active_frns[asterix::highest_frn];
                uint16_t active_count = 0;
                if (!parse_fspec(raw_data, raw_offset, block_end_raw, active_frns, active_count))
                {
                    // TODO: FSpec in valid, what now? discard everything? probably only current parsed record
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                if (internal_data->get_capacity() < out_offset + sizeof(record))
                {
                    // Out buffer too small for additional record, TODO: resize
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                out_offset += sizeof(record);

                // At this point we have a valid record
                record_count++;

                // Reserve space for record
                uint32_t record_off = out_offset;
                out_offset += sizeof(record);

                uint32_t record_all_item_size = active_uap->item_count * sizeof(item);

                // Save the offset where the child items for the current record will be located
                uint32_t record_child_items_offset = out_offset + active_uap->highest_frn * sizeof(item);

                if (internal_data->get_capacity() < out_offset + record_all_item_size)
                {
                    // Out buffer too small for all items, TODO: resize
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                uint32_t cur_item_offset = out_offset;
                uint32_t item_count      = active_count;

                for (uint16_t i = 1; i < active_uap->highest_frn; ++i)
                {
                    cur_item_offset += sizeof(item);

                    // Default set item to not populated
                    internal_data->at<item>(cur_item_offset)->populated = false;

                    const field_spec* spec = active_uap->get_spec(i);

                    // Current UAP may not have a spec for frn i
                    if (!spec)
                        continue;

                    auto old_childs = record_child_items_offset;

                    if (!parse_item(*spec, raw_data, raw_offset, block_end_raw, internal_data, cur_item_offset, record_child_items_offset))
                    {
                        // TODO do we return here? just output a log? IDK
                        continue;
                    }
 
                    // Calculate added child items - Could've just passed a child count param, but parse_item already has enough params
                    if (record_child_items_offset > old_childs)
                        item_count += (record_child_items_offset - old_childs) / sizeof(item);

                    internal_data->at<item>(cur_item_offset)->populated = true;
                }

                internal_data->at<record>(record_off)->item_count = item_count;
            }
       
            internal_data->at<block>(block_off)->record_count = record_count;
        }

        internal_data->at<frame>(frame_off)->block_count = block_count;
        internal_data->set_size(out_offset);

        return true;
    }
}