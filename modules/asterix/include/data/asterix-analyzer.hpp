#pragma once

/**
 * @file    asterix-analyzer.hpp
 * @author  dexus1337
 * @brief   Defines the analyzer for the asterix data format.
 * @version 1.0
 * @date    11.07.2026
 */

#include <adam-sdk.hpp>

#include "api/api-asterix.hpp"

namespace adam::modules::asterix
{
    /**
     * @class asterix_analyzer
     * @brief Analyzes raw asterix buffers and extracts tabular data for the GUI inspector.
     */
    class ADAM_ASTERIX_API asterix_analyzer : public adam::analyzer
    {
    public:
        asterix_analyzer();
        ~asterix_analyzer() override = default;

        /** @brief Analyzes the data in the buffer and populates the result vector. */
        bool analyze(const class adam::buffer* buf, std::vector<row>& results) const override;

        /** @brief Lazily generates expansions for a specific row in the buffer. */
        bool analyze_expanded(const uint8_t* data, size_t size, const uint8_t* ref_data, size_t ref_size, size_t row_idx, std::vector<expanded_data>& out_expansions) const override;

    private:
    };
}
