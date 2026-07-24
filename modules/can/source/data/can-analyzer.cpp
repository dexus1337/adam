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
    }

    bool can_analyzer::analyze(const adam::buffer* buf, std::vector<row>& results) const
    {
        if (!buf) return false;

        const uint8_t* current = static_cast<const uint8_t*>(buf->get_data());
        const uint8_t* end = current + buf->get_size();

        while (current + sizeof(can_message) <= end)
        {
            const can_message* msg = reinterpret_cast<const can_message*>(current);
            uint8_t len = msg->get_length();
            if (current + len > end) break; // Ensure full message fits in buffer

            row r;
            r.columns.push_back(std::format("0x{:04X}", msg->get_id())); // ID
            r.columns.push_back(std::to_string(msg->get_data_length())); // Length
            
            std::string data_str;
            for (int j = 0; j < msg->get_data_length(); ++j)
            {
                if (j > 0) data_str += " ";
                data_str += std::format("{:02X}", msg->get_data()[j]);
            }
            r.columns.push_back(data_str);                                      // Data

            results.push_back(std::move(r));
            current += len;
        }

        return true;
    }

    bool can_analyzer::analyze_expanded(const uint8_t* data, size_t size, const uint8_t* ref_data, size_t ref_size, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        (void)ref_data;
        (void)ref_size;
        
        if (!data) return false;

        const uint8_t* current = data;
        const uint8_t* end = current + size;

        size_t current_row = 0;
        const can_message* target_msg = nullptr;

        while (current + sizeof(can_message) <= end)
        {
            const can_message* m = reinterpret_cast<const can_message*>(current);
            uint8_t len = m->get_length();
            if (current + len > end) break;

            if (current_row == row_idx)
            {
                target_msg = m;
                break;
            }

            current += len;
            current_row++;
        }

        if (!target_msg) return false;

        const can_message& msg = *target_msg;

        adam::analyzer::expanded_data ed;
        ed.data_type = adam::analyzer::expanded_data::type_text;

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

                int chunk_end = std::min((int)msg.get_data_length(), chunk_start + 4);

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

        ed.text_content = text;
        
        out_expansions.push_back(std::move(ed));

        return true;
    }
}
