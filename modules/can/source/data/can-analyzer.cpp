#include "data/can-analyzer.hpp"
#include "data/can-types.hpp"
#include <cstdint>
#include <string>
#include <cstdio>
#include <format>
#include <bitset>
namespace adam::modules::can
{
    can_analyzer::can_analyzer()
    {
        m_columns = 
        {
            "Frame ID",
            "Timestamp",
            "ID",
            "Length",
            "Data",
        };

        m_column_types =
        {
            column_frame_id,
            column_timestamp,
            column_text,
            column_text,
            column_text
        };

        m_b_row_expandable = true;

        m_expandable_columns =
        {
            "FRN",
            "Item",
            "Data"
        };
    }

    bool can_analyzer::analyze(const adam::buffer* buf, std::vector<row>& results) const
    {
        if (!buf) return false;

        const can_message* messages = reinterpret_cast<const can_message*>(buf->get_data());
        uint32_t message_count = buf->get_size() / sizeof(can_message);

        // Iterate over blocks
        for (uint32_t i = 0; i < message_count; ++i)
        {
            row r;
            r.columns.push_back(std::format("0x{:04X}", messages[i].get_id())); // ID
            r.columns.push_back(std::to_string(messages[i].get_data_length())); // Length
            
            std::string data_str;
            for (int j = 0; j < messages[i].get_data_length(); ++j)
            {
                if (j > 0) data_str += " ";
                data_str += std::format("{:02X}", messages[i].data[j]);
            }
            r.columns.push_back(data_str);                                      // Data

            results.push_back(std::move(r));
        }

        return true;
    }

    bool can_analyzer::analyze_expanded(const uint8_t* data, size_t size, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        if (!data) return false;

        const can_message* messages = reinterpret_cast<const can_message*>(data);
        uint32_t message_count = size / sizeof(can_message);

        if (row_idx >= message_count) return false;

        const can_message& msg = messages[row_idx];

        adam::analyzer::expanded_data ed;
        ed.data_type = adam::analyzer::expanded_data::type_text;

        std::string text;

        for (int chunk_start = 0; chunk_start < msg.get_data_length(); chunk_start += 4)
        {
            std::string line_byte_nr = "Byte Nr:    ";
            std::string line_byte    = "Byte:       ";
            std::string line_bit_nr  = "Bit Nr:     ";
            std::string line_bits    = "Bits:       ";

            int chunk_end = std::min((int)msg.get_data_length(), chunk_start + 4);

            for (int i = chunk_start; i < chunk_end; ++i)
            {
                line_byte_nr += std::format("{:<13}", i);
                line_byte    += std::format("{:<13}", std::format("0x{:02X}", msg.data[i]));
                line_bit_nr  += std::format("{:<5}{:<8}", i * 8, i * 8 + 4);
                
                std::string bits_str = std::format("{} {}", 
                    std::bitset<4>((msg.data[i] >> 4) & 0x0F).to_string(), 
                    std::bitset<4>(msg.data[i] & 0x0F).to_string());
                line_bits    += std::format("{:<13}", bits_str);
            }

            text += line_byte_nr + "\n" + line_byte + "\n" + line_bit_nr + "\n" + line_bits + "\n";
            if (chunk_end < msg.get_data_length())
            {
                text += "\n";
            }
        }

        ed.text_content = text;
        
        out_expansions.push_back(std::move(ed));

        return true;
    }
}
