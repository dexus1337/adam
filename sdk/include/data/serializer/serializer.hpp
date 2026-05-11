#pragma once

/**
 * @file    serializer.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format serializers, providing a common interface for serializing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"


namespace adam 
{
    /**
     * @class serializer
     * @brief A base class for data format serializers, providing a common interface for serializing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API serializer 
    {
    public:

        /** @brief Constructs a new serializer object. */
        serializer();

        /** @brief Destroys the serializer object and cleans up resources. */
        ~serializer();

    };
}