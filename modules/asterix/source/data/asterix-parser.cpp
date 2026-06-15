#include "data/asterix-parser.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include "data/uap/cat048.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix::data
{
    // Forward declaration of recursive helper
    static bool parse_uap(const asterix_uap& uap, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset);

    static bool parse_fspec(const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, uint8_t* active_frns, uint16_t& active_count)
    {
        active_count = 0;
        bool fx = true;
        uint8_t byte_index = 0;

        while (fx)
        {
            if (raw_offset >= raw_length) return false;
            
            uint8_t fspec_byte = raw_data[raw_offset++];
            
            for (int bit = 7; bit >= 1; --bit)
            {
                if ((fspec_byte >> bit) & 1)
                {
                    uint16_t frn = static_cast<uint16_t>((byte_index * 7) + (8 - bit));
                    if (frn < 256)
                    {
                        active_frns[active_count++] = static_cast<uint8_t>(frn);
                    }
                }
            }
            
            fx = (fspec_byte & 1) != 0;
            byte_index++;
        }
        return true;
    }

    static bool parse_item(const asterix_field_spec& spec, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint32_t item_start_offset = raw_offset;
        auto* out_ptr = internal_data->begin_as<uint8_t>();
        
        switch (spec.type)
        {
            case asterix_item_type::FIXED:
            {
                if (raw_offset + spec.length > raw_length) return false;
                raw_offset += spec.length;
                
                if (internal_data->get_capacity() < out_offset + sizeof(asterix_item_header)) return false;
                
                new (out_ptr + out_offset) asterix_item_header(item_start_offset, spec.length, asterix_item_type::FIXED);
                out_offset += sizeof(asterix_item_header);
                break;
            }
            case asterix_item_type::VARIABLE:
            {
                bool fx = true;
                while (fx)
                {
                    if (raw_offset >= raw_length) return false;
                    fx = (raw_data[raw_offset++] & 1) != 0;
                }
                uint16_t total_len = static_cast<uint16_t>(raw_offset - item_start_offset);
                
                if (internal_data->get_capacity() < out_offset + sizeof(asterix_item_header)) return false;
                
                new (out_ptr + out_offset) asterix_item_header(item_start_offset, total_len, asterix_item_type::VARIABLE);
                out_offset += sizeof(asterix_item_header);
                break;
            }
            case asterix_item_type::REPETITIVE:
            {
                if (raw_offset >= raw_length) return false;
                uint8_t rep = raw_data[raw_offset++];
                // For standard repetitive, length is usually strictly defined, but here we assume a fixed block length or nested if we support it.
                // For simplicity, we just assume spec.length defines the size of 1 block, though sometimes we need nested sub-UAPs.
                // If spec.sub_uap is not null, it's a complex repetitive item.

                if (internal_data->get_capacity() < out_offset + sizeof(asterix_repetitive_item) + (rep * sizeof(uint32_t))) return false;

                auto* rep_item = new (out_ptr + out_offset) asterix_repetitive_item(item_start_offset, 0, rep);
                uint32_t* child_offsets = rep_item->get_child_offsets();
                out_offset += sizeof(asterix_repetitive_item) + (rep * sizeof(uint32_t));

                for (uint8_t i = 0; i < rep; ++i)
                {
                    child_offsets[i] = out_offset;
                    if (spec.sub_uap)
                    {
                        if (!parse_uap(*spec.sub_uap, raw_data, raw_offset, raw_length, internal_data, out_offset)) return false;
                    }
                    else
                    {
                        if (raw_offset + spec.length > raw_length) return false;
                        raw_offset += spec.length;
                        
                        if (internal_data->get_capacity() < out_offset + sizeof(asterix_item_header)) return false;
                        new (out_ptr + out_offset) asterix_item_header(raw_offset - spec.length, spec.length, asterix_item_type::FIXED);
                        out_offset += sizeof(asterix_item_header);
                    }
                }
                rep_item->header.data_length = static_cast<uint16_t>(raw_offset - item_start_offset);
                break;
            }
            case asterix_item_type::COMPOUND:
            {
                if (!spec.sub_uap) return false; // Compound must have sub_uap

                // The FSPEC length is not known upfront, it depends on FX bits
                uint8_t active_frns[256];
                uint16_t active_count = 0;
                
                if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns, active_count)) return false;

                if (internal_data->get_capacity() < out_offset + sizeof(asterix_compound_item) + (active_count * sizeof(uint32_t))) return false;

                auto* comp_item = new (out_ptr + out_offset) asterix_compound_item(item_start_offset, 0, active_count);
                uint32_t* child_offsets = comp_item->get_child_offsets();
                out_offset += sizeof(asterix_compound_item) + (active_count * sizeof(uint32_t));

                for (uint16_t i = 0; i < active_count; ++i)
                {
                    const asterix_field_spec* sub_spec = spec.sub_uap->get_spec(active_frns[i]);
                    child_offsets[i] = out_offset;
                    
                    if (sub_spec)
                    {
                        if (!parse_item(*sub_spec, raw_data, raw_offset, raw_length, internal_data, out_offset)) return false;
                    }
                    else
                    {
                        // Missing sub-spec definition, cannot reliably parse past a compound item without knowing length!
                        return false; 
                    }
                }
                comp_item->header.data_length = static_cast<uint16_t>(raw_offset - item_start_offset);
                break;
            }
            case asterix_item_type::EXPLICIT:
            {
                if (raw_offset >= raw_length) return false;
                uint8_t explicit_len = raw_data[raw_offset]; // length includes this byte
                if (raw_offset + explicit_len > raw_length) return false;
                
                if (spec.sub_uap)
                {
                    // Skip the 1-byte length indicator for the FSPEC parsing
                    raw_offset += 1;
                    
                    uint8_t active_frns[256];
                    uint16_t active_count = 0;
                    
                    // Parse bounded by the explicit field's length
                    if (!parse_fspec(raw_data, raw_offset, item_start_offset + explicit_len, active_frns, active_count)) return false;

                    if (internal_data->get_capacity() < out_offset + sizeof(asterix_explicit_item) + (active_count * sizeof(uint32_t))) return false;

                    auto* exp_item = new (out_ptr + out_offset) asterix_explicit_item(item_start_offset, explicit_len, active_count);
                    uint32_t* child_offsets = exp_item->get_child_offsets();
                    out_offset += sizeof(asterix_explicit_item) + (active_count * sizeof(uint32_t));

                    for (uint16_t i = 0; i < active_count; ++i)
                    {
                        const asterix_field_spec* sub_spec = spec.sub_uap->get_spec(active_frns[i]);
                        child_offsets[i] = out_offset;
                        
                        if (sub_spec)
                        {
                            if (!parse_item(*sub_spec, raw_data, raw_offset, item_start_offset + explicit_len, internal_data, out_offset)) return false;
                        }
                        else
                        {
                            // Unrecognized sub-item, cannot reliably parse past it without lengths
                            return false; 
                        }
                    }
                    
                    // Force advance to the end of the explicit block in case child items were truncated or mis-measured
                    raw_offset = item_start_offset + explicit_len;
                }
                else
                {
                    // Standard raw explicit item (unparsed payload)
                    raw_offset += explicit_len;
                    
                    if (internal_data->get_capacity() < out_offset + sizeof(asterix_explicit_item)) return false;
                    
                    new (out_ptr + out_offset) asterix_explicit_item(item_start_offset, explicit_len, 0);
                    out_offset += sizeof(asterix_explicit_item);
                }
                break;
            }
        }
        return true;
    }

    static bool parse_uap(const asterix_uap& uap, const uint8_t* raw_data, uint32_t& raw_offset, uint32_t raw_length, adam::buffer* internal_data, uint32_t& out_offset)
    {
        uint8_t active_frns[256];
        uint16_t active_count = 0;

        if (!parse_fspec(raw_data, raw_offset, raw_length, active_frns, active_count)) return false;

        for (uint16_t i = 0; i < active_count; ++i)
        {
            uint8_t frn = active_frns[i];
            const asterix_field_spec* spec = uap.get_spec(frn);
            
            // Note: we can also check for uap.get_expansion(frn) here to attach metadata to the tree.

            if (spec)
            {
                if (!parse_item(*spec, raw_data, raw_offset, raw_length, internal_data, out_offset)) return false;
            }
            else
            {
                // Unrecognized FRN, we can't safely advance
                return false;
            }
        }
        return true;
    }

    bool asterix_parser::parse(class adam::buffer* buf, class adam::buffer*& internal_data)
    {
        if (!buf)
            return false;

        internal_data = adam::buffer_manager::get().request_buffer(4096);
        if (!internal_data)
            return false;

        internal_data->set_referenced_buffer(buf);

        uint32_t out_offset = 0;
        uint32_t raw_offset = 0;
        const uint8_t* raw_data = buf->get_begin_as<uint8_t>();
        uint32_t raw_length = buf->get_size();

        if (raw_offset >= raw_length) return false;
        
        uint8_t cat = raw_data[raw_offset++];
        
        const asterix_uap* active_uap = uap_pool::get_instance().get_uap(cat);
        if (!active_uap)
        {
            adam::buffer_manager::get().return_buffer(internal_data);
            return false; // Unsupported category
        }
        
        // For Asterix, length field is next
        if (raw_offset + 2 > raw_length) 
        {
            adam::buffer_manager::get().return_buffer(internal_data);
            return false;
        }
        
        uint16_t total_length = (raw_data[raw_offset] << 8) | raw_data[raw_offset + 1];
        raw_offset += 2;
        
        if (total_length > raw_length || total_length < 3)
        {
            adam::buffer_manager::get().return_buffer(internal_data);
            return false;
        }
        
        // Parse the main UAP bounded by the packet's total_length
        if (!parse_uap(*active_uap, raw_data, raw_offset, total_length, internal_data, out_offset))
        {
            adam::buffer_manager::get().return_buffer(internal_data);
            return false;
        }

        internal_data->set_size(out_offset);
        return true;
    }
}
