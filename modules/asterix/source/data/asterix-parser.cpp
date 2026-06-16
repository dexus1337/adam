#include "data/asterix-parser.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    static inline bool parse_fixed_item(        const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static inline bool parse_variable_item(     const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static inline bool parse_repetitive_item(   const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static inline bool parse_compound_item(     const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static inline bool parse_explicit_item(     const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static inline bool parse_item(              const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);
    static        bool parse_explicit_fspec(    const uap* sub_uap,     const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, uint8_t* active_frns, uint16_t& active_count);

    static bool parse_fspec(const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, uint8_t* active_frns, uint16_t& active_count)
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
                    if (frn >= 256) return false; // Bug #3: FRN overflow is malformed data, not a silent skip
                    active_frns[active_count++] = static_cast<uint8_t>(frn);
                }
            }
            octet_idx++;
        }

        raw_offset += octet_idx;
        return true;
    }

    static inline bool parse_explicit_fspec(const uap* sub_uap, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, uint8_t* active_frns, uint16_t& active_count)
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
        (void)raw_data;

        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();

        if (raw_offset + spec.primary_header_size > raw_length) return false;
        raw_offset += spec.primary_header_size;

        if (internal_data->get_capacity() < out_offset + sizeof(item)) return false;

        new (out_ptr + out_offset) item(item_start_offset, spec.primary_header_size, item_type_fixed);
        out_offset += sizeof(item);
        return true;
    }

    static inline bool parse_variable_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();
        uint16_t first_size = spec.primary_header_size;
        uint16_t extent_size = spec.extent_size;

        if (raw_offset + first_size > raw_length) return false;
        raw_offset += first_size;
        bool fx = (raw_data[raw_offset - 1] & 1) != 0;

        while (fx)
        {
            if (raw_offset + extent_size > raw_length) return false;
            raw_offset += extent_size;
            fx = (raw_data[raw_offset - 1] & 1) != 0;
        }
        uint16_t total_len = static_cast<uint16_t>(raw_offset - item_start_offset);

        if (internal_data->get_capacity() < out_offset + sizeof(item)) return false;

        new (out_ptr + out_offset) item(item_start_offset, total_len, item_type_variable);
        out_offset += sizeof(item);
        return true;
    }

    static inline bool parse_repetitive_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();
        uint8_t ind_len = spec.extent_size;
        if (ind_len == 0 || raw_offset + ind_len > raw_length) return false;

        uint16_t rep = 0;
        if (ind_len == 1)
        {
            rep = raw_data[raw_offset];
        }
        else if (ind_len == 2)
        {
            rep = adam::swap_2(raw_data + raw_offset);
        }
        else
        {
            return false;
        }
        raw_offset += ind_len;

        if (rep > 4096) return false;

        if (internal_data->get_capacity() < out_offset + sizeof(repetitive_item) + (rep * sizeof(uint32_t))) return false;

        auto* rep_item = new (out_ptr + out_offset) repetitive_item(item_start_offset, 0, rep);
        uint32_t* child_offsets = rep_item->get_child_offsets();
        out_offset += sizeof(repetitive_item) + (rep * sizeof(uint32_t));

        for (uint16_t i = 0; i < rep; ++i)
        {
            child_offsets[i] = out_offset;
            if (raw_offset + spec.primary_header_size > raw_length) return false;
            raw_offset += spec.primary_header_size;

            if (internal_data->get_capacity() < out_offset + sizeof(item)) return false;
            new (out_ptr + out_offset) item(raw_offset - spec.primary_header_size, spec.primary_header_size, item_type_fixed);
            out_offset += sizeof(item);
        }
        rep_item->header.data_length = static_cast<uint16_t>(raw_offset - item_start_offset);
        return true;
    }

    static inline bool parse_compound_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        if (!spec.sub_uap) return false; // Compound must have sub_uap
        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();

        uint8_t active_frns[256];
        uint16_t active_count = 0;
        if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns, active_count)) return false;

        if (internal_data->get_capacity() < out_offset + sizeof(compound_item) + (active_count * sizeof(uint32_t))) return false;
        auto* comp_item = new (out_ptr + out_offset) compound_item(item_start_offset, 0, active_count);
        uint32_t* child_offsets = comp_item->get_child_offsets();
        out_offset += sizeof(compound_item) + (active_count * sizeof(uint32_t));

        for (uint16_t i = 0; i < active_count; ++i)
        {
            const field_spec* sub_spec = spec.sub_uap->get_spec(active_frns[i]);
            child_offsets[i] = out_offset;
            if (sub_spec)
            {
                if (!parse_item(*sub_spec, raw_data, raw_offset, raw_length, internal_data, out_offset)) return false;
            }
            else
            {
                return false; // Missing sub-spec definition
            }
        }
        comp_item->header.data_length = static_cast<uint16_t>(raw_offset - item_start_offset);
        return true;
    }

    static inline bool parse_explicit_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
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

    static inline bool parse_item(const field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
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
                return parse_compound_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset);
            case item_type_explicit:
                return parse_explicit_item(spec, raw_data, raw_offset, raw_length, internal_data, out_offset);
            default:
                return false;
        }
    }

    bool asterix_parser::parse(class adam::buffer* buf, class adam::buffer*& internal_data)
    {
        if (!buf)
            return false;

        uint32_t raw_length = buf->get_size();
        const uint8_t* raw_data = buf->get_begin_as<uint8_t>();

        uint32_t max_blocks = raw_length / sizeof(raw_block_header) + 1;
        uint32_t scratch_size = max_blocks * sizeof(uint32_t);

        uint32_t required_capacity = raw_length * 32 + sizeof(frame) + max_blocks * sizeof(uint32_t) + scratch_size;
        internal_data = adam::buffer_manager::get().request_buffer(required_capacity);
        if (!internal_data)
            return false;

        internal_data->set_referenced_buffer(buf);

        uint8_t* out_ptr = internal_data->begin_as<uint8_t>();

        // Reserve space for frame + max_blocks offsets
        uint32_t out_offset = sizeof(frame) + max_blocks * sizeof(uint32_t);

        uint32_t* block_offsets = reinterpret_cast<uint32_t*>(out_ptr + required_capacity - scratch_size);
        uint32_t block_count = 0;

        // Bug #5: The scratch area lives at the top of the buffer. out_offset must never reach it.
        const uint32_t out_limit = required_capacity - scratch_size;

        uint32_t raw_offset = 0;

        while (raw_offset < raw_length)
        {
            uint32_t block_start_raw = raw_offset;

            if (raw_offset + sizeof(raw_block_header) > raw_length)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                return false;
            }

            const auto* raw_header = reinterpret_cast<const raw_block_header*>(raw_data + raw_offset);
            const uap* active_uap = raw_header->get_uap();
            if (!active_uap)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                return false;
            }

            uint16_t total_length = raw_header->get_length();
            if (total_length < minimum_block_length || block_start_raw + total_length > raw_length)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                return false;
            }

            raw_offset += sizeof(raw_block_header);

            uint32_t block_end_raw = block_start_raw + total_length;

            uint32_t record_offsets[1024];
            uint32_t record_count = 0;

            // Parse records in the block
            while (raw_offset < block_end_raw)
            {
                if (record_count >= 1024)
                {
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                uint32_t record_start_raw = raw_offset;

                uint8_t active_frns[256];
                uint16_t active_count = 0;
                if (!parse_fspec(raw_data, raw_offset, block_end_raw, active_frns, active_count))
                {
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                uint32_t record_offset = out_offset;
                uint32_t record_header_size = sizeof(record) + active_count * sizeof(uint32_t);

                if (internal_data->get_capacity() < out_offset + record_header_size)
                {
                    adam::buffer_manager::get().return_buffer(internal_data);
                    return false;
                }

                out_offset += record_header_size;

                auto* rec_ptr = new (out_ptr + record_offset) record(active_count, 0, record_start_raw);
                uint32_t* item_offsets = rec_ptr->get_item_offsets();

                for (uint16_t i = 0; i < active_count; ++i)
                {
                    const field_spec* spec = active_uap->get_spec(active_frns[i]);
                    if (!spec)
                    {
                        adam::buffer_manager::get().return_buffer(internal_data);
                        return false;
                    }

                    item_offsets[i] = out_offset;

                    // Bug #5: Ensure out_offset hasn't encroached into the scratch area.
                    if (out_offset >= out_limit)
                    {
                        adam::buffer_manager::get().return_buffer(internal_data);
                        return false;
                    }

                    if (!parse_item(*spec, raw_data, raw_offset, block_end_raw, internal_data, out_offset))
                    {
                        adam::buffer_manager::get().return_buffer(internal_data);
                        return false;
                    }
                }

                rec_ptr->record_length = raw_offset - record_start_raw;
                record_offsets[record_count++] = record_offset;
            }

            // Write block header at the end of its records
            uint32_t block_offset = out_offset;
            uint32_t block_header_size = sizeof(block) + record_count * sizeof(uint32_t);

            if (internal_data->get_capacity() < out_offset + block_header_size)
            {
                adam::buffer_manager::get().return_buffer(internal_data);
                return false;
            }

            out_offset += block_header_size;

            auto* blk_ptr = new (out_ptr + block_offset) block(
                raw_header->category,
                static_cast<uint16_t>(record_count),
                total_length,
                block_start_raw
            );
            uint32_t* rec_offsets_dest = blk_ptr->get_record_offsets();
            for (uint32_t r = 0; r < record_count; ++r)
            {
                rec_offsets_dest[r] = record_offsets[r];
            }

            block_offsets[block_count++] = block_offset;
        }

        // Write buffer frame header at offset 0
        auto* buf_header = new (out_ptr) frame(static_cast<uint16_t>(block_count));
        uint32_t* block_offsets_dest = buf_header->get_block_offsets();
        for (uint32_t b = 0; b < block_count; ++b)
        {
            block_offsets_dest[b] = block_offsets[b];
        }

        internal_data->set_size(out_offset);
        return true;
    }
}