#pragma once

/**
 * @file    filter.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data filters, providing a common interface for filtering data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "data/processor/data-processor.hpp"


namespace adam 
{
    /**
     * @class filter
     * @brief A base class for data filters, providing a common interface for filtering data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API filter : public data_processor   
    {
    public:

        /**
         * @brief Constructs a new filter object.
         */
        filter();

        /**
         * @brief Destroys the filter object and cleans up resources.
         */
        ~filter();

    };
}