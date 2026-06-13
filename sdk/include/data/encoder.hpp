#pragma once

/**
 * @file    encoder.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format encoders, providing a common interface for encoding data from different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"


namespace adam 
{
    class buffer;

    /**
     * @class encoder
     * @brief A base class for data format encoders, providing a common interface for encoding data from different formats used in the ADAM system.
     */
    class ADAM_SDK_API encoder 
    {

    public:

        /** @brief Destroys the encoder object and cleans up resources. */
        virtual ~encoder() = default;

        virtual bool encode(class buffer*& buf, class buffer* internal_data) = 0;

    protected:

        /** @brief Constructs a new encoder object. */
        encoder() = default;

    };
}