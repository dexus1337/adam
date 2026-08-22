#pragma once

/**
 * @file    filter-frame-aligner.hpp
 * @author  dexus1337
 * @brief   Defines a filter that aligns frames in a buffer.
 * @version 1.0
 * @date    14.06.2026
 */

 
#include "api/api-sdk.hpp"

#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "data/processors/filter.hpp"   


namespace adam 
{
    /**
     * @class filter_frame_aligner
     * @brief A filter that aligns frames in a buffer.
     */
    class ADAM_SDK_API filter_frame_aligner : public filter   
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "filter_frame_aligner"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        filter_frame_aligner(const string_hashed& item_name);

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };
        
        virtual bool handle_data(buffer*& buf) override;

    protected:

        configuration_parameter_integer* m_byte_offset;

    };
}