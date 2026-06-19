#include "data/asterix-encoder.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include "data/asterix-internal.hpp"
#include <adam-sdk.hpp>
#include <cstring>

namespace adam::modules::asterix
{
    static inline void encode_item(adam::buffer* out_buf, uint32_t& out_offset, const item* itm, const uap* sub_uap, adam::buffer* ref_buf)
    {
        if (!itm->is_populated()) return;

        if (!itm->is_modified() && ref_buf)
        {
            out_buf->fill_data(ref_buf->get_at<uint8_t>(itm->raw_offset), itm->raw_length, out_offset);
            out_offset += itm->raw_length;
            return;
        }

        switch (itm->type)
        {
            case item_type_fixed:
            case item_type_variable:
            case item_type_repetetive:
            {
                if (ref_buf)
                {
                    out_buf->fill_data(ref_buf->get_at<uint8_t>(itm->raw_offset), itm->raw_length, out_offset);
                    out_offset += itm->raw_length;
                }
                break;
            }
            case item_type_compound:
            {
                if (!sub_uap) return;

                // 1. Determine active children FRNs
                bool active_frns[asterix::highest_frn] = {false};
                uint8_t highest_active_frn = 0;
                for (uint16_t i = 1; i <= sub_uap->get_highest_frn(); ++i)
                {
                    const auto* child = itm->get_child_item(i);
                    if (child && child->is_populated())
                    {
                        active_frns[i] = true;
                        highest_active_frn = static_cast<uint8_t>(i);
                    }
                }

                // 2. Write compound FSPEC (multiple octets, FX in bit 0)
                uint8_t needed_octets = (highest_active_frn + 6) / 7;
                if (needed_octets == 0) needed_octets = 1;

                for (uint8_t oct = 0; oct < needed_octets; ++oct)
                {
                    uint8_t val = 0;
                    for (int bit = 7; bit >= 1; --bit)
                    {
                        uint16_t frn = static_cast<uint16_t>(oct * 7 + (8 - bit));
                        if (frn <= highest_active_frn && active_frns[frn])
                        {
                            val |= (1 << bit);
                        }
                    }
                    if (oct < needed_octets - 1)
                    {
                        val |= 1;
                    }
                    out_buf->fill_data(&val, 1, out_offset);
                    out_offset += 1;
                }

                // 3. Write children
                for (uint16_t i = 1; i <= sub_uap->get_highest_frn(); ++i)
                {
                    if (active_frns[i])
                    {
                        const auto* child = itm->get_child_item(i);
                        const auto* child_spec = sub_uap->get_spec(static_cast<uint8_t>(i));
                        encode_item(out_buf, out_offset, child, child_spec ? child_spec->sub_uap : nullptr, ref_buf);
                    }
                }
                break;
            }
            case item_type_explicit:
            {
                if (sub_uap)
                {
                    uint32_t explicit_start = out_offset;
                    out_offset += 1; // leave space for length byte

                    // 1. Determine active children FRNs
                    bool active_frns[asterix::highest_frn] = {false};
                    uint8_t highest_active_frn = 0;
                    for (uint16_t i = 1; i <= sub_uap->get_highest_frn(); ++i)
                    {
                        const auto* child = itm->get_child_item(i);
                        if (child && child->is_populated())
                        {
                            active_frns[i] = true;
                            highest_active_frn = static_cast<uint8_t>(i);
                        }
                    }

                    // 2. Write explicit FSPEC (All 8 bits are data bits, no FX bit!)
                    uint8_t fspec_octets = (sub_uap->get_highest_frn() + 7) / 8;
                    for (uint8_t oct = 0; oct < fspec_octets; ++oct)
                    {
                        uint8_t val = 0;
                        for (int bit = 7; bit >= 0; --bit)
                        {
                            uint16_t frn = static_cast<uint16_t>(oct * 8 + (8 - bit));
                            if (frn <= highest_active_frn && active_frns[frn])
                            {
                                val |= (1 << bit);
                            }
                        }
                        out_buf->fill_data(&val, 1, out_offset);
                        out_offset += 1;
                    }

                    // 3. Write children
                    for (uint16_t i = 1; i <= sub_uap->get_highest_frn(); ++i)
                    {
                        if (active_frns[i])
                        {
                            const auto* child = itm->get_child_item(i);
                            const auto* child_spec = sub_uap->get_spec(static_cast<uint8_t>(i));
                            encode_item(out_buf, out_offset, child, child_spec ? child_spec->sub_uap : nullptr, ref_buf);
                        }
                    }

                    // Fill length byte
                    uint32_t total_explicit_len = out_offset - explicit_start;
                    uint8_t len_val = static_cast<uint8_t>(total_explicit_len);
                    out_buf->fill_data(&len_val, 1, explicit_start);
                }
                else
                {
                    if (ref_buf)
                    {
                        out_buf->fill_data(ref_buf->get_at<uint8_t>(itm->raw_offset), itm->raw_length, out_offset);
                        out_offset += itm->raw_length;
                    }
                }
                break;
            }
        }
    }

    static inline void encode_record(adam::buffer* out_buf, uint32_t& out_offset, const record* rec, adam::buffer* ref_buf)
    {
        const uap* active_uap = rec->find_used_uap();
        if (!active_uap) return;

        // 1. Determine active FRNs
        bool active_frns[asterix::highest_frn] = {false};
        uint8_t highest_active_frn = 0;
        for (uint16_t i = 1; i <= active_uap->get_highest_frn(); ++i)
        {
            const auto* itm = rec->get_item(static_cast<uint8_t>(i));
            if (itm && itm->is_populated())
            {
                active_frns[i] = true;
                highest_active_frn = static_cast<uint8_t>(i);
            }
        }

        // 2. Write record FSPEC (multiple octets, FX in bit 0)
        uint8_t needed_octets = (highest_active_frn + 6) / 7;
        if (needed_octets == 0) needed_octets = 1;

        for (uint8_t oct = 0; oct < needed_octets; ++oct)
        {
            uint8_t val = 0;
            for (int bit = 7; bit >= 1; --bit)
            {
                uint16_t frn = static_cast<uint16_t>(oct * 7 + (8 - bit));
                if (frn <= highest_active_frn && active_frns[frn])
                {
                    val |= (1 << bit);
                }
            }
            if (oct < needed_octets - 1)
            {
                val |= 1;
            }
            out_buf->fill_data(&val, 1, out_offset);
            out_offset += 1;
        }

        // 3. Write items
        for (uint16_t i = 1; i <= active_uap->get_highest_frn(); ++i)
        {
            if (active_frns[i])
            {
                const auto* itm = rec->get_item(static_cast<uint8_t>(i));
                const auto* spec = active_uap->get_spec(static_cast<uint8_t>(i));
                encode_item(out_buf, out_offset, itm, spec ? spec->sub_uap : nullptr, ref_buf);
            }
        }
    }

    bool asterix_encoder::encode(class adam::buffer*& buf, class adam::buffer* internal_data)
    {
        if (!internal_data) return false;

        const auto* frm = internal_data->get_begin_as<frame>();
        if (!frm) return false;

        adam::buffer* ref_buf = internal_data->get_referenced_buffer();

        // Optimization: if nothing in the frame has been modified, we can reuse the original buffer directly
        if (!frm->is_modified() && ref_buf)
        {
            buf = ref_buf;
            buf->add_ref();
            return true;
        }

        uint32_t capacity = ref_buf ? ref_buf->get_capacity() : 65536;
        buf = adam::buffer_manager::get().request_buffer(capacity);
        if (!buf) return false;

        uint32_t out_offset = 0;

        for (uint16_t b = 0; b < frm->block_count; ++b)
        {
            const block* blk = frm->get_block(b);
            if (!blk) continue;

            if (!blk->is_modified() && ref_buf)
            {
                // Fast path: copy entire unmodified block
                buf->fill_data(ref_buf->get_at<uint8_t>(blk->raw_offset), blk->raw_length, out_offset);
                out_offset += blk->raw_length;
            }
            else
            {
                // Write Block Header
                uint32_t block_start_offset = out_offset;

                // Write Category
                buf->fill_data(&blk->category, 1, out_offset);
                out_offset += 1;

                // Write placeholder for length (2 bytes)
                uint16_t zero_len = 0;
                buf->fill_data(&zero_len, 2, out_offset);
                out_offset += 2;

                // Write records
                for (uint16_t r = 0; r < blk->record_count; ++r)
                {
                    const record* rec = blk->get_record(r);
                    if (!rec) continue;

                    if (!rec->is_modified() && ref_buf)
                    {
                        // Fast path: copy entire unmodified record
                        buf->fill_data(ref_buf->get_at<uint8_t>(rec->raw_offset), rec->raw_length, out_offset);
                        out_offset += rec->raw_length;
                    }
                    else
                    {
                        encode_record(buf, out_offset, rec, ref_buf);
                    }
                }

                // Update final block length in big-endian
                uint16_t block_len = static_cast<uint16_t>(out_offset - block_start_offset);
                uint16_t be_block_len = adam::swap_2(reinterpret_cast<const uint8_t*>(&block_len));
                buf->fill_data(&be_block_len, 2, block_start_offset + 1);
            }
        }

        buf->set_size(out_offset);
        return true;
    }
}
