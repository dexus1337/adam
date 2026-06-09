#pragma once

/**
 * @file    rff.hpp
 * @author  dexus1337
 * @brief   Structures for the rff file format.
 * @version 1.0
 * @date    04.06.2026
 */


#include "api/api-recrep.hpp"
#include "api/api-sdk.hpp"

#include <adam-sdk.hpp>
#include <cstdint>
#include <ctime>


namespace adam::modules::recrep
{
    namespace rff
    {
        #pragma pack(push, 1)
        template<size_t length>
        struct text_field
        {
            char text[length]   = {};
            char terminator     = '\0';
        };
        #pragma pack(pop)

        static ADAM_CONSTEXPR const char* time_format_string = "%m/%d/%y %H:%M:%S"; // MM/dd/yy hh:mm:ss
        using time_string        = text_field<19>;
        using ref_name_string    = text_field<12>;
        using user_string        = text_field<11>;
        using system_id_string   = text_field<11>;
        using data_source_string = text_field<16>;
        using interface_string   = text_field<5>;

        ADAM_RECREP_API time_string to_time_string(const std::tm& tm_val);
        ADAM_RECREP_API std::tm     from_time_string(const time_string& ts);
        
        #pragma pack(push, 1)
        struct version_string
        {
            char v_literal      = 'V';
            char major          = '1';
            char dot_literal    = '.';
            char minor          = '1';
            char terminator     = '\0';
        };
        #pragma pack(pop)
        
        #pragma pack(push, 1)
        struct file_header
        {
            static const uint64_t magic_number = 0x0020202020464652ull; //52 46 46 20 20 20 20 00 -> "RFF   ."

            time_string         time_start          = time_string();
            time_string         time_end            = time_string();
            ref_name_string     ref_name            = ref_name_string();
            version_string      version             = version_string();
            user_string         user                = user_string();
            uint32_t            file_size_bytes     = 0;
            char                reserved[6]         = {};
            uint64_t            magic               = magic_number;
            uint8_t             file_format         = 2;
            uint8_t             data_format         = 0;
            system_id_string    system_id           = system_id_string();
            uint8_t             unit_id             = 0;
            uint8_t             interface_type      = 0;
            data_source_string  data_source         = data_source_string();
            interface_string    interface           = interface_string();
            uint8_t             protocol            = 0;
            uint16_t            channel_id          = 0;
            int32_t             speed               = 0;
        };
        #pragma pack(pop)

        #pragma pack(push, 1)
        struct block_header
        {
            uint32_t        time_to_start_ms;   // ms since start of file
            uint16_t        block_size_bytes;
        };
        #pragma pack(pop)
    }
}