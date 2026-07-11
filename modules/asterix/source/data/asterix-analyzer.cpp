#include "data/asterix-analyzer.hpp"
#include "data/asterix-parser.hpp"
#include "data/asterix-internal.hpp"
#include "data/asterix-uap.hpp"
#include <string>

namespace adam::modules::asterix
{
    asterix_analyzer::asterix_analyzer()
    {
        m_columns = {
            "Category",
            "UAP",
            "Items",
            "Size",
            "Offset"
        };
    }

    const adam::analyzer::columns& asterix_analyzer::get_columns() const
    {
        return m_columns;
    }

    bool asterix_analyzer::analyze(const adam::buffer* buf, std::vector<row>& results) const
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

                row r;
                r.push_back(std::to_string(rec.category));
                r.push_back(used_uap ? used_uap->get_name().c_str() : "Unknown");
                r.push_back(std::to_string(rec.item_count));
                r.push_back(std::to_string(rec.raw_length));
                r.push_back(std::to_string(rec.raw_offset));

                results.push_back(std::move(r));
            }
        }

        return true;
    }
}
