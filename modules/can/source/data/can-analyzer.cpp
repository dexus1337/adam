#include "data/can-analyzer.hpp"
#include "data/can-types.hpp"
#include <cstdint>
#include <string>
#include <cstdio>

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

        const auto* root_frame = reinterpret_cast<const frame*>(buf->get_data());

        // Iterate over blocks
        for (auto block_it = root_frame->begin(); block_it != root_frame->end(); ++block_it)
        {
            // Iterate over records in the block
            for (auto rec_it = block_it->begin(); rec_it != block_it->end(); ++rec_it)
            {
                const auto& rec = *rec_it;
                const auto* used_uap = rec.find_used_uap();

                uint8_t items = 0;

                for (auto it = rec.begin(); it != rec.end(); ++it)
                    if (it->is_populated()) items++;

                const raw_sac_sic* sac_sic = nullptr;

                if (used_uap && used_uap->get_sacsic_frn() > 0)
                {
                    const item* sac_sic_item = rec.get_item(used_uap->get_sacsic_frn());
                    if (sac_sic_item && sac_sic_item->is_populated())
                        sac_sic = buf->get_at<raw_sac_sic>(sac_sic_item->raw_offset);
                }

                row r;
                r.columns.push_back(used_uap ? used_uap->get_name().c_str() : "Unknown");                                  // UAP
                r.columns.push_back("");                                                                                   // Type
                r.columns.push_back(std::string(sac_sic ? std::format("{:03d}/{:03d}", sac_sic->sac, sac_sic->sic) : "")); // SAC/SIC
                r.columns.push_back(std::to_string(items));                                                                // Items
                r.columns.push_back(std::to_string(rec.raw_length));                                                       // Size

                results.push_back(std::move(r));
            }
        }

        return true;
    }

    bool can_analyzer::analyze_expanded(const uint8_t* data, size_t size, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        if (!data) return false;

        const auto* root_frame = reinterpret_cast<const frame*>(data);

        size_t current_row = 0;
        for (auto block_it = root_frame->begin(); block_it != root_frame->end(); ++block_it)
        {
            for (auto rec_it = block_it->begin(); rec_it != block_it->end(); ++rec_it)
            {
                if (current_row == row_idx)
                {
                    const auto& rec = *rec_it;
                    const auto* used_uap = rec.find_used_uap();

                    adam::analyzer::expanded_data ed;
                    ed.data_type = adam::analyzer::expanded_data::type_table;

                    auto process_item = [&](auto& self, const item* current_item, uint8_t current_frn, const uap* current_uap, int indent_level) -> void
                    {
                        if (!current_item->is_populated()) return;

                        row sub_r;
                        std::string indent_str(indent_level * 2, ' ');
                        sub_r.columns.push_back(indent_str + std::to_string(current_frn));
                        
                        const field_spec* spec = current_uap ? current_uap->get_spec(current_frn) : nullptr;

                        if (spec)
                            sub_r.columns.push_back(indent_str + spec->name);
                        else
                            sub_r.columns.push_back(indent_str + "Unknown");

                        std::string hex_data;
                        if (current_item->raw_offset + current_item->raw_length <= size)
                        {
                            const uint8_t* item_data = data + current_item->raw_offset;
                            for (size_t i = 0; i < current_item->raw_length; ++i)
                            {
                                char buf_str[4];
                                snprintf(buf_str, sizeof(buf_str), "%02X ", item_data[i]);
                                hex_data += buf_str;
                            }
                            if (!hex_data.empty()) hex_data.pop_back();
                        }
                        sub_r.columns.push_back(std::move(hex_data));

                        ed.table_rows.push_back(std::move(sub_r));

                        if (current_item->child_count > 0)
                        {
                            for (uint8_t child_frn = 1; child_frn <= current_item->child_count; ++child_frn)
                            {
                                const item* child = current_item->get_child_item(child_frn);
                                if (child)
                                {
                                    self(self, child, child_frn, spec ? spec->sub_uap : nullptr, indent_level + 1);
                                }
                            }
                        }
                    };

                    uint8_t frn = 1;
                    for (auto it = rec.begin(); it != rec.end() && (!used_uap || frn <= used_uap->get_highest_frn()); ++it, ++frn)
                    {
                        process_item(process_item, &(*it), frn, used_uap, 0);
                    }

                    out_expansions.push_back(std::move(ed));
                    return true;
                }
                current_row++;
            }
        }

        return false;
    }
}
