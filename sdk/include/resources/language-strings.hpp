#pragma once

/**
 * @file    language-stirngs.hpp
 * @author  dexus1337
 * @brief   This file hosts all functions to get readable text from all types of status strings, in languages englisch and german
 * @version 1.0
 * @date    08.05.2026
 */


#include <string>

#include "controller/controller.hpp"
#include "controller/response.hpp"


namespace adam
{
    /**
     * @class   language_strings
     * @brief   A static class which hosts all strings in the ADAM system in supported languages
     */
    class language_strings
    {
    public:
        static std::string_view get_controller_status_text(controller::status stat, language lang);
        
        static std::string_view get_response_type_text(response::type typ, language lang);

        static std::string_view success_message(language lang);
        static std::string_view unknown_type_message(std::string_view type, int val, language lang);
    };
}