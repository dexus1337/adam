#pragma once

/**
 * @file    message-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs used in messages between commander and controller
 * @version 1.0
 * @date    14.05.2026
 */

#include "api/sdk-api.hpp"
#include "types/string-hashed.hpp"
#include "resources/language.hpp"
#include "commander/messages/command.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace adam 
{
    static constexpr size_t max_name_length = 64;
    static constexpr size_t max_port_type_length = 64;
    static constexpr size_t max_path_length = 384;

    namespace messages
    {
        struct initial_data_header
        {
            language_info lang_info;
            struct module_base_info
            {
                uint32_t    module_paths            = 0;
                uint32_t    available_modules       = 0;
                uint32_t    unavailable_modules     = 0;
                uint32_t    loaded_modules          = 0;
            } mod_info;
        };
        static_assert(sizeof(initial_data_header) <= command::get_max_data_length(), "initial_data_header exceeds maximum command data size");

        struct module_path_data
        {
            uint32_t idx;
            char path[max_path_length];

            void setup(const char* p, uint32_t i)
            {
                std::strncpy(path, p, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
                idx = i;
            }
        };
        static_assert(sizeof(module_path_data) <= command::get_max_data_length(), "module_path_data exceeds maximum command data size");

        struct module_path_remove_data
        {
            uint32_t idx;
        };
        static_assert(sizeof(module_path_remove_data) <= command::get_max_data_length(), "module_path_remove_data exceeds maximum command data size");

        struct module_action_data
        {
            char module_name[max_name_length];

            void setup(const char* n)
            {
                std::strncpy(module_name, n, sizeof(module_name) - 1);
                module_name[sizeof(module_name) - 1] = '\0';
            }
        };
        static_assert(sizeof(module_action_data) <= command::get_max_data_length(), "module_action_data exceeds maximum command data size");

        struct port_destroy_data
        {
            string_hashed::hash_datatype port;
        };
        static_assert(sizeof(port_destroy_data) <= command::get_max_data_length(), "port_destroy_data exceeds maximum command data size");

        struct port_action_data
        {
            string_hashed::hash_datatype port;
        };
        static_assert(sizeof(port_action_data) <= command::get_max_data_length(), "port_action_data exceeds maximum command data size");

        struct connection_action_data
        {
            string_hashed::hash_datatype connection;
        };
        static_assert(sizeof(connection_action_data) <= command::get_max_data_length(), "connection_action_data exceeds maximum command data size");

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
        static_assert(sizeof(connection_destroy_data) <= command::get_max_data_length(), "connection_destroy_data exceeds maximum command data size");

        struct connection_rename_data
        {
            string_hashed::hash_datatype connection;
            char new_name[max_name_length];
        };
        static_assert(sizeof(connection_rename_data) <= command::get_max_data_length(), "connection_rename_data exceeds maximum command data size");
    }
}