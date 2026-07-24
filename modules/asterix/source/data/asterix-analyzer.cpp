#include "data/asterix-analyzer.hpp"
#include "data/asterix-parser.hpp"
#include "data/asterix-internal.hpp"
#include "data/asterix-types.hpp"
#include "data/asterix-uap.hpp"
#include <cstdint>
#include <string>
#include <cstdio>

namespace adam::modules::asterix
{
    asterix_analyzer::asterix_analyzer()
    {
        m_columns = 
        {
            "Frame ID",
            "Timestamp",
            "CAT",
            "Type",
            "SAC/SIC",
            "Items",
            "Size"
        };

        m_column_types =
        {
            column_frame_id,    // Frame ID
            column_timestamp,   // Timestamp
            column_text,        // CAT
            column_text,        // Type
            column_text,        // SAC/SIC
            column_text,        // Items
            column_text,        // Size
        };

        m_column_fonts =
        {
            column_font_normal, // Frame ID
            column_font_normal, // Timestamp
            column_font_normal, // CAT
            column_font_normal, // Type
            column_font_normal, // SAC/SIC
            column_font_normal, // Items
            column_font_normal  // Size
        };

        m_column_weights = 
        {
            0.1f,               // Frame ID
            0.15f,              // Timestamp
            0.225f,             // CAT
            0.225f,             // Type
            0.1f,               // SAC/SIC
            0.1f,               // Items
            0.1f,               // Size
        };

        m_b_row_expandable = true;

        m_expandable_columns =
        {
            "FRN",
            "Item",
            "Data"
        };

        m_expandable_columns_fonts = 
        {
            column_font_normal,
            column_font_normal,
            column_font_mono
        };

        m_expandable_columns_weights = 
        {
            0.1f,
            0.45f,
            0.45f,
        };

    }

    bool asterix_analyzer::analyze(const adam::buffer* buf, std::vector<row>& results) const
    {
        if (!buf) return false;

        const auto* root_frame = reinterpret_cast<const frame*>(buf->get_data());

        // Iterate over blocks
        for (const auto& block : *root_frame)
        {
            if (block.record_count == 0)
            {
                row r;
                r.columns.push_back(std::format("CAT{:03} (No UAP)", block.category));  // UAP
                r.columns.push_back("");                                                // Type
                r.columns.push_back("");                                                // SAC/SIC
                r.columns.push_back("");                                                // Items
                r.columns.push_back(std::to_string(block.raw_length));                  // Size

                results.push_back(std::move(r));
            }
            else
            {
                // Iterate over records in the block
                for (const auto& rec : block)
                {
                    const auto* used_uap = rec.find_used_uap();
                    const auto* raw_data = buf->get_referenced_buffer();
                    const auto* sac_sic  = used_uap ? used_uap->get_record_sac_sic(&rec, raw_data) : nullptr;

                    uint8_t items = 0;

                    for (auto it = rec.begin(); it != rec.end(); ++it)
                        if (it->is_populated()) items++;

                    row r;
                    r.columns.push_back(used_uap ? used_uap->get_name().c_str() : "Unknown");                                   // UAP
                    r.columns.push_back(used_uap ? std::string(used_uap->get_record_type_name(&rec, raw_data)) : "");           // Type
                    r.columns.push_back(std::string(sac_sic ? std::format("{:03d}/{:03d}", sac_sic->sac, sac_sic->sic) : ""));  // SAC/SIC
                    r.columns.push_back(std::to_string(items));                                                                 // Items
                    r.columns.push_back(std::to_string(rec.raw_length));                                                        // Size

                    results.push_back(std::move(r));
                }
            }
        }

        return true;
    }

    bool asterix_analyzer::analyze_expanded(const uint8_t* data, size_t size, const uint8_t* ref_data, size_t ref_size, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        if (!data) return false;

        const auto* root_frame = reinterpret_cast<const frame*>(data);

        size_t current_row = 0;
        for (const auto& block : *root_frame)
        {
            if (block.record_count == 0)
            {
                current_row++;
                continue;
            }

            for (const auto& rec : block)
            {
                if (current_row != row_idx)
                {
                    current_row++;
                    continue;
                }

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
                    if (ref_data && current_item->raw_offset + current_item->raw_length <= ref_size)
                    {
                        const uint8_t* item_data = ref_data + current_item->raw_offset;
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
        }

        return false;
    }
}
