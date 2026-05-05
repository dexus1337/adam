#pragma once

/**
 * @file    data-processor.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "configuration/configuration-item.hpp"

namespace adam 
{
    /**
     * @class data_processor
     * @brief A base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API data_processor : public configuration_item
    {
    public:

        /**
         * @brief Constructs a new data processor object.
         */
        data_processor() = default;

        /**
         * @brief Destroys the data processor object and cleans up resources.
         */
        ~data_processor() = default;

    };
}