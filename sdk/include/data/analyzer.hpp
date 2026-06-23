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

        using columns = std::vector<std::string>;

        /** @brief Destroys the analyzer object and cleans up resources. */
        virtual ~analyzer() = default;

        /** @brief Returns the column names of the data format. */
        virtual const columns&  get_columns() const                                     = 0;

        /** @brief Analyzes the data in the buffer and returns the result as a CSV string. */
        virtual bool            analyze(class buffer* buf, std::string& result) const   = 0;

    protected:

        /** @brief Constructs a new analyzer object. */
        analyzer() = default;

    };
}