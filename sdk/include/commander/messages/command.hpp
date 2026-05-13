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

        acquire_initial_data,

        set_language,

        port_create,
        port_set_parameter,
        port_destroy,

        inspector_create,
        inspector_destroy,

        connection_create,
        connection_destroy
    };

    /**
     * @class command
     * @brief Defines a command for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API command : public commander_message_template<command_type>
    {
    public:
        struct initial_data_header
        {
            language_info lang_info;
            struct module_base_info
            {
                uint32_t available_modules      = 0;
                uint32_t unavailable_modules    = 0;
                uint32_t loaded_modules         = 0;
            } mod_info;
        };
        static_assert(sizeof(initial_data_header) <= command::get_max_data_length(), "initial_data_header exceeds maximum command data size");

        struct port_destroy_data
        {
            string_hashed::hash_datatype port;
        };

        struct port_parameter_updated_data
        {
            string_hashed::hash_datatype port;
            string_hashed::hash_datatype parameter;
        };
        static_assert(sizeof(port_parameter_updated_data) <= command::get_max_data_length(), "port_parameter_updated_data exceeds maximum command data size");
        struct inspector_create_data
        {
            string_hashed::hash_datatype port;
        };
        static_assert(sizeof(inspector_create_data) <= command::get_max_data_length(), "inspector_create_data exceeds maximum command data size");

        struct inspector_destroy_data
        {
            string_hashed::hash_datatype port;
        };
        static_assert(sizeof(inspector_destroy_data) <= command::get_max_data_length(), "inspector_destroy_data exceeds maximum command data size");

        struct connection_destroy_data
        {
            string_hashed::hash_datatype connection;
        };

        /** @brief Constructs a new command object.*/
        command(command_type t = command_type::invalid) : commander_message_template(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;
    };
    #pragma pack(pop)
}