#include "data/can-analyzer.hpp"
#include "data/can-types.hpp"
#include "data/can-profile.hpp"
#include <cstdint>
#include <string>
#include <cstdio>
#include <format>
#include <bitset>
#include <algorithm>

/**
 * @file    can-analyzer.cpp
 * @author  dexus1337
 * @brief   Implements the CAN analyzer with profile signal decoding and expandable inspector view.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can
{
    can_analyzer::can_analyzer()
    {
        m_columns = 
        {
            "Frame ID",
            "Timestamp",
            "ID",
            "ECU",
            "Name",
            "DLC",
            "Signals",
            "Data"
        };

        m_column_types =
        {
            column_frame_id,    // Frame ID
            column_timestamp,   // Timestamp
            column_text,        // ID
            column_text,        // ECU
            column_text,        // Name
            column_text,        // DLC
            column_text,        // Signals
            column_text         // Data
        };

        m_column_fonts =
        {
            column_font_normal, // Frame ID
            column_font_normal, // Timestamp
            column_font_mono,   // ID
            column_font_normal, // ECU
            column_font_normal, // Name
            column_font_normal, // DLC
            column_font_normal, // Signals
            column_font_mono    // Data
        };

        m_column_weights = 
        {
            0.08f,              // Frame ID
            0.12f,              // Timestamp
            0.09f,              // ID
            0.11f,              // ECU
            0.18f,              // Name
            0.06f,              // DLC
            0.06f,              // Signals
            0.30f               // Data
        };

        m_b_row_expandable = true;

        m_expandable_columns =
        {
            "Index",
            "Signal",
            "Description",
            "Bit Range",
            "Raw (Hex)",
            "Raw (Dec)"
        };

        m_expandable_columns_fonts =
        {
            column_font_normal, // Index
            column_font_normal, // Signal
            column_font_normal, // Description
            column_font_mono,   // Bit Range
            column_font_mono,   // Raw (Hex)
            column_font_mono    // Raw (Dec)
        };

        m_expandable_columns_weights =
        {
            0.08f,              // Index
            0.18f,              // Signal
            0.38f,              // Description
            0.14f,              // Bit Range
            0.11f,              // Raw (Hex)
            0.11f               // Raw (Dec)
        };
    }

    const can_profile* can_analyzer::get_profile() const
    {
        return can_profile_pool::get().get_default_profile();
    }

    bool can_analyzer::analyze(const adam::buffer* buf, std::vector<row>& results) const
    {
        if (!buf)
        {
            return false;
        }

        const uint8_t* current = buf->get_begin_as<uint8_t>();
        const uint8_t* end = current + buf->get_size();
        const auto* profile = get_profile();

        while (current + sizeof(can_message) <= end)
        {
            const auto* msg = reinterpret_cast<const can_message*>(current);
            uint8_t len = msg->get_length();
            if (current + len > end)
            {
                break;
            }

            uint32_t msg_id = msg->get_id();
            bool is_ext = msg->id.bits.is_extended;
            const auto* msg_spec = profile ? profile->find_message(msg_id, is_ext) : nullptr;

            row r;
            if (is_ext)
            {
                r.columns.push_back(std::format("0x{:08X}", msg_id));
            }
            else
            {
                r.columns.push_back(std::format("0x{:04X}", msg_id));
            }

            r.columns.push_back(msg_spec ? msg_spec->ecu_name : "");
            r.columns.push_back(msg_spec ? msg_spec->name : "");
            r.columns.push_back(std::to_string(msg->get_data_length()));
            r.columns.push_back(msg_spec ? std::to_string(msg_spec->signal_count) : "0");

            std::string data_str;
            for (int j = 0; j < msg->get_data_length(); ++j)
            {
                if (j > 0)
                {
                    data_str += " ";
                }
                data_str += std::format("{:02X}", msg->get_data()[j]);
            }
            r.columns.push_back(std::move(data_str));

            results.push_back(std::move(r));
            current += len;
        }

        return true;
    }

    bool can_analyzer::analyze_expanded(const uint8_t* data, size_t size, const uint8_t* /*ref_data*/, size_t /*ref_size*/, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        if (!data)
        {
            return false;
        }

        const uint8_t* current = data;
        const uint8_t* end = current + size;

        size_t current_row = 0;
        const can_message* target_msg = nullptr;

        while (current + sizeof(can_message) <= end)
        {
            const auto* m = reinterpret_cast<const can_message*>(current);
            uint8_t len = m->get_length();
            if (current + len > end)
            {
                break;
            }

            if (current_row == row_idx)
            {
                target_msg = m;
                break;
            }

            current += len;
            current_row++;
        }

        if (!target_msg)
        {
            return false;
        }

        const can_message& msg = *target_msg;
        const auto* profile = get_profile();
        const auto* msg_spec = profile ? profile->find_message(msg.get_id(), msg.id.bits.is_extended) : nullptr;

        if (msg_spec && msg_spec->signal_count > 0)
        {
            adam::analyzer::expanded_data ed;
            ed.data_type = adam::analyzer::expanded_data::type_table;

            for (size_t i = 0; i < msg_spec->signal_count; ++i)
            {
                const auto& sig = msg_spec->signals[i];
                uint64_t raw_val = extract_raw_signal(msg.get_data(), msg.get_data_length(), sig);

                row sub_r;
                sub_r.columns.push_back(std::to_string(sig.index));
                sub_r.columns.push_back(sig.name);
                sub_r.columns.push_back(sig.description);
                sub_r.columns.push_back(std::format("Bits [{}:{}]", sig.bit_offset, sig.bit_offset + sig.bit_length - 1));
                sub_r.columns.push_back(std::format("0x{:X}", raw_val));
                sub_r.columns.push_back(std::to_string(raw_val));

                ed.table_rows.push_back(std::move(sub_r));
            }

            out_expansions.push_back(std::move(ed));
        }

        // Also add bit layout text inspector
        adam::analyzer::expanded_data text_ed;
        text_ed.data_type = adam::analyzer::expanded_data::type_text;

        std::string text;
        if (msg.get_data_length() == 0)
        {
            text = "No data bytes (DLC = 0)";
        }
        else
        {
            for (int chunk_start = 0; chunk_start < msg.get_data_length(); chunk_start += 4)
            {
                std::string line_byte_nr = "Byte Nr:    ";
                std::string line_byte    = "Byte:       ";
                std::string line_bit_nr  = "Bit Nr:     ";
                std::string line_bits    = "Bits:       ";

                int chunk_end = std::min(static_cast<int>(msg.get_data_length()), chunk_start + 4);

                for (int i = chunk_start; i < chunk_end; ++i)
                {
                    line_byte_nr += std::format("{:<13}", i);
                    line_byte    += std::format("{:<13}", std::format("0x{:02X}", msg.get_data()[i]));
                    line_bit_nr  += std::format("{:<5}{:<8}", i * 8, i * 8 + 4);
                    
                    std::string bits_str = std::format("{} {}", 
                        std::bitset<4>((msg.get_data()[i] >> 4) & 0x0F).to_string(), 
                        std::bitset<4>(msg.get_data()[i] & 0x0F).to_string());
                    line_bits    += std::format("{:<13}", bits_str);
                }

                text += line_byte_nr + "\n" + line_byte + "\n" + line_bit_nr + "\n" + line_bits + "\n";
                if (chunk_end < msg.get_data_length())
                {
                    text += "\n";
                }
            }
        }

        text_ed.text_content = std::move(text);
        out_expansions.push_back(std::move(text_ed));

        return true;
    }
}
