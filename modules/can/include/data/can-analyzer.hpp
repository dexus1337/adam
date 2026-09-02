#pragma once

/**
 * @file    can-analyzer.hpp
 * @author  dexus1337
 * @brief   Defines the analyzer for the CAN data format with profile-aware decoding.
 * @version 1.0
 * @date    24.08.2026
 */

#include <adam-core.hpp>

#include "api/api-can.hpp"
#include "data/can-profile.hpp"

namespace adam::modules::can
{
    /**
     * @class can_analyzer
     * @brief Analyzes raw CAN buffers and extracts tabular data and expandable signal details.
     */
    class ADAM_CAN_API can_analyzer : public adam::analyzer
    {
    public:
        can_analyzer(const can_profile* profile = nullptr);
        ~can_analyzer() override = default;

        inline const can_profile*                           get_profile() const                         { return m_profile ? m_profile : can_profile_pool::get().get_default_profile(); }

        /** @brief Analyzes the data in the buffer and populates the result vector. */
        bool                                                analyze(const class adam::buffer* buf, std::vector<row>& results) const override;

        /** @brief Lazily generates expansions for a specific row in the buffer. */
        bool                                                analyze_expanded(const uint8_t* data, size_t size, const uint8_t* ref_data, size_t ref_size, size_t row_idx, std::vector<expanded_data>& out_expansions) const override;

        inline void                                         set_profile(const can_profile* profile)     { m_profile = profile; }

    private:
        const can_profile*                                  m_profile;
    };
}
