#pragma once

/**
 * @file    data-processor.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include "data/processor.hpp"

namespace adam 
{
    /**
     * @class converter
     * @brief A base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API converter : public data_processor
    {
    public: 
        /** @brief Destroys the data processor object and cleans up resources. */
        virtual ~converter();

    protected:

        /** @brief Constructs a new data processor object. */
        converter(const string_hashed& item_name);

    };
}