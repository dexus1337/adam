#include "data/analyzer.hpp"

namespace adam
{

    bool analyzer::analyze_expanded(const uint8_t* data, size_t size, const uint8_t* ref_data, size_t ref_size, size_t row_idx, std::vector<expanded_data>& out_expansions) const
    {
        (void)data;
        (void)size;
        (void)ref_data;
        (void)ref_size;
        (void)row_idx;
        (void)out_expansions;

        return false;
    }

}
