#pragma once

/**
 * @file    encoder.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format encoders, providing a common interface for encoding data from different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
#include "configuration/configuration-item.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam 
{
    class buffer;

    /**
     * @class encoder
     * @brief A base class for data format encoders, providing a common interface for encoding data from different formats used in the ADAM system.
     */
    class ADAM_SDK_API encoder : public configuration_item
    {
    public:

        static const configuration_parameter_list& get_default_parameters();

        virtual ~encoder() = default;

        virtual bool encode(class buffer*& buf, class buffer* internal_data) = 0;

    protected:

        encoder(const string_hashed& item_name = "encoder"_ct, const configuration_parameter_list& default_params = get_default_parameters());

    };
}