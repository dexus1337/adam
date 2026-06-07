#pragma once

/**
 * @file    command.hpp
 * @author  dexus1337
 * @brief   Defines a command for the controller
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api-sdk.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "type.hpp"


namespace adam 
{
    
    enum command_type : uint16_t
    {
        invalid = 0,

        login,
        logout,

        acquire_initial_data,

        set_language,

        module_path_add,
        module_path_remove,
        module_scan,
        module_load,
        module_unload,

        port_create,
        port_set_parameter,
        port_destroy,
        port_start,
        port_stop,
        port_rename,
        port_inject_data,
        connection_set_input_data_format,
        connection_set_output_data_format,

        connection_create,
        connection_destroy,
        connection_start,
        connection_stop,
        connection_rename,
        connection_port_add,
        connection_port_remove,
        connection_sorting_index_change,
        connection_color_change,

        connection_input_inspector_create,
        connection_input_inspector_destroy,
        connection_output_inspector_create,
        connection_output_inspector_destroy,
        inspector_create,
        inspector_destroy,

    };

    /**
     * @class command
     * @brief Defines a command for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API command : public commander_message_template<command_type>
    {
    public:
        /** @brief Constructs a new command object.*/
        command(command_type t = command_type::invalid) : commander_message_template(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;
    };
    #pragma pack(pop)
}