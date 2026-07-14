#pragma once

/**
 * @file    analyzer.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format analyzer. Used to make data human readable and analyzable in a CSV format.
 * @version 1.0
 * @date    23.06.2026
 */

 
#include "api/api-sdk.hpp"

#include <string>
#include <vector>
#include <cstdint>

namespace adam 
{
    class buffer;

    /**
     * @class   analyzer
     * @brief   Defines a base class for data format analyzer. Used to make data human readable and analyzable in a CSV format.
     */
    class ADAM_SDK_API analyzer 
    {
    public:

        enum column_type
        {
            column_frame_id,
            column_timestamp,
            column_text
        };

        struct row;

        struct expanded_data
        {
            enum type
            {
                type_text,
                type_table
            };

            type data_type;
            std::string text_content;
            std::vector<row> table_rows;
        };

        struct row
        {
            std::vector<std::string> columns;
            std::vector<expanded_data> expansions;
            bool expansions_fetched = false;
        };

        virtual ~analyzer() = default;

        /** @brief Analyzes the data in the buffer and populates the result vector with multiple rows (one string per column). */
        virtual bool analyze(const class buffer* buf, std::vector<row>& results) const = 0;

        /** @brief Lazily generates expansions for a specific row in the buffer. */
        virtual bool analyze_expanded(const uint8_t* data, size_t size, size_t row_idx, std::vector<expanded_data>& out_expansions) const;

        bool                            is_row_expandable()      const { return m_b_row_expandable; }
        const std::vector<std::string>& get_columns()            const { return m_columns; }
        const std::vector<column_type>& get_column_types()       const { return m_column_types; }
        const std::vector<std::string>& get_expandable_columns() const { return m_expandable_columns; }

    protected:

        analyzer() = default;

        std::vector<std::string> m_columns;
        std::vector<column_type> m_column_types;

        bool                     m_b_row_expandable;

        std::vector<std::string> m_expandable_columns;
    };
}