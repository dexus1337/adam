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

#include "type.hpp"


namespace adam 
{
    
    enum class command_type : uint8_t
    {
        invalid = 0,

        login,
        logout,
        set_language,

        inspector_create,
        inspector_destroy
    };

    /**
     * @class command
     * @brief Defines a command for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API command : public command_response_type<command_type>
    {
    public:

        struct inspector_create_data
        {
            string_hashed::hash_datatype port;
        };

        struct inspector_destroy_data
        {
            string_hashed::hash_datatype port;
        };

        /** @brief Constructs a new command object.*/
        command(command_type t = command_type::invalid) : command_response_type(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;
    };
    #pragma pack(pop)
}