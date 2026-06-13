#pragma once

/**
 * @file    asterix-to-text-converter.hpp
 * @author  dexus1337
 * @brief   Defines the asterix to text converter
 * @version 1.0
 * @date    10.06.2026
 */


#include "api/api-asterix.hpp"
#include "memory/buffer/buffer.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::asterix
{
    class ADAM_ASTERIX_API asterix_to_text_converter : public adam::converter
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "asterix-to-text-converter"_ct; }

        asterix_to_text_converter(const string_hashed& name);

        ~asterix_to_text_converter() = default;

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; }; /**< THANK YOU MSCV for your really good constexp eval. Thats why we have to do such beautiful things */
        
        virtual bool handle_data(buffer*& buf) override;
    };
}