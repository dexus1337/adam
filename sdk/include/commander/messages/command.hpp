#pragma once

/**
 * @file    command.hpp
 * @author  dexus1337
 * @brief   Defines a command for the controller
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "type.hpp"
#include "resources/language.hpp"


namespace adam 
{
    
    enum class command_type : uint16_t
    {
        invalid = 0,

        login,
        logout,

        receive_initial_data,

        set_language,

        inspector_create,
        inspector_destroy
    };

    /**
     * @class command
     * @brief Defines a command for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API command : public commander_message_template<command_type>
    {
    public:

        struct initial_data
        {
            struct header
            {
                struct language_info
                {
                    language lang                = language_english;                                    /**< The current language of the controller. */
                    uint32_t supported_languages = (1 << language_english) | (1 << language_german);    /**< Bitfield of supported languages, indexed by the language enum value. */
                } lang_info;

                struct module_base_info
                {
                    uint32_t available_modules      = 0;
                    uint32_t unavailable_modules    = 0;
                    uint32_t loaded_modules         = 0;
                } mod_info;
            };

            struct module_info
            {
                enum module_status : uint8_t
                {
                    available = 0,
                    unavailable = 1,
                    loaded = 2
                } status;
                char name[64];      /**< The name of the module. */
                char path[256];     /**< The file path to the module's shared library. */
                uint32_t version;   /**< The version of the module. */

                void setup(module_status s, const char* n, const char* p, uint32_t v)
                {
                    status = s;
                    std::strncpy(name, n, sizeof(name) - 1);
                    name[sizeof(name) - 1] = '\0';
                    std::strncpy(path, p, sizeof(path) - 1);
                    path[sizeof(path) - 1] = '\0';
                    version = v;
                }
            };
        };

        struct inspector_create_data
        {
            string_hashed::hash_datatype port;
        };

        struct inspector_destroy_data
        {
            string_hashed::hash_datatype port;
        };

        /** @brief Constructs a new command object.*/
        command(command_type t = command_type::invalid) : commander_message_template(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;
    };
    #pragma pack(pop)
}