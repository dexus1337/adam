#pragma once

/**
 * @file    message-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs used in messages between commander and controller
 * @version 1.0
 * @date    14.05.2026
 */

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "resources/language.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/event.hpp"
#include "configuration/parameters/configuration-parameter.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace adam 
{
    static ADAM_CONSTEXPR size_t max_name_length         = 64;
    static ADAM_CONSTEXPR size_t max_port_type_length    = 64;
    static ADAM_CONSTEXPR size_t max_path_length         = 384;
    static ADAM_CONSTEXPR size_t max_description_length  = 349;

    enum data_direction : uint8_t
    {
        data_direction_in   = 1,
        data_direction_out  = 0,
    };

    namespace messages
    {
        #pragma pack(push, 1)

        struct initial_data_header
        {
            language_info lang_info;

            struct module_base_info
            {
                uint32_t module_paths            = 0;
                uint32_t available_modules       = 0;
                uint32_t unavailable_modules     = 0;
                uint32_t loaded_modules          = 0;
            } mod_info;

            struct connections_info
            {
                uint32_t ports                  = 0;
                uint32_t processors             = 0;
                uint32_t connections            = 0;
            } conn_info;

            struct configs_base_info
            {
                char     name[max_name_length];
                char     description[max_description_length];
                uint32_t paths_count            = 0;
                uint32_t available_configs      = 0;
            } cfg_info;
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
            string_hash port;
        };
        static_assert(sizeof(port_destroy_data) <= command::get_max_data_length(), "port_destroy_data exceeds maximum command data size");

        struct port_action_data
        {
            string_hash port;
        };
        static_assert(sizeof(port_action_data) <= command::get_max_data_length(), "port_action_data exceeds maximum command data size");

        struct port_inject_data
        {
            string_hash     port;
            uint32_t        total_size;
            uint16_t        size;
            data_direction  direction;
            uint8_t         data[command::get_max_data_length() - sizeof(string_hash) - sizeof(uint32_t) - sizeof(uint16_t) - sizeof(data_direction)];
        };
        static_assert(sizeof(port_inject_data) <= command::get_max_data_length(), "port_inject_data exceeds maximum command data size");

        struct port_rename_data
        {
            string_hash port;
            char new_name[max_name_length];
            uint64_t edited;
        };
        static_assert(sizeof(port_rename_data) <= command::get_max_data_length(), "port_rename_data exceeds maximum command data size");

        struct processor_destroy_data
        {
            string_hash processor;
        };
        static_assert(sizeof(processor_destroy_data) <= command::get_max_data_length(), "processor_destroy_data exceeds maximum command data size");

        struct processor_action_data
        {
            string_hash processor;
        };
        static_assert(sizeof(processor_action_data) <= command::get_max_data_length(), "processor_action_data exceeds maximum command data size");

        struct processor_rename_data
        {
            string_hash processor;
            char new_name[max_name_length];
            uint64_t edited;
        };
        static_assert(sizeof(processor_rename_data) <= command::get_max_data_length(), "processor_rename_data exceeds maximum command data size");

        struct processor_parameter_updated_data
        {
            string_hash processor;
            configuration_parameter::view param_view;
            uint8_t data[command::get_max_data_length() - sizeof(string_hash) - sizeof(configuration_parameter::view)];
        };
        static_assert(sizeof(processor_parameter_updated_data) <= command::get_max_data_length(), "processor_parameter_updated_data exceeds maximum command data size");

        struct processor_set_parameter_data
        {
            string_hash processor;
            configuration_parameter::view param_view;
            uint8_t data[command::get_max_data_length() - sizeof(string_hash) - sizeof(configuration_parameter::view)];
        };
        static_assert(sizeof(processor_set_parameter_data) <= command::get_max_data_length(), "processor_set_parameter_data exceeds maximum command data size");

        struct connection_data_format_data
        {
            string_hash connection;
            string_hash format;
            string_hash format_module;
            bool        valid_chain;
        };
        static_assert(sizeof(connection_data_format_data) <= command::get_max_data_length(), "connection_data_format_data exceeds maximum command data size");

        struct connection_action_data
        {
            string_hash connection;
        };
        static_assert(sizeof(connection_action_data) <= command::get_max_data_length(), "connection_action_data exceeds maximum command data size");

        struct port_parameter_updated_data
        {
            string_hash port;
            configuration_parameter::view param_view;
            uint8_t data[command::get_max_data_length() - sizeof(string_hash) - sizeof(configuration_parameter::view)];
        };
        static_assert(sizeof(port_parameter_updated_data) <= command::get_max_data_length(), "port_parameter_updated_data exceeds maximum command data size");

        struct port_set_parameter_data
        {
            string_hash port;
            configuration_parameter::view param_view;
            uint8_t data[command::get_max_data_length() - sizeof(string_hash) - sizeof(configuration_parameter::view)];
        };
        static_assert(sizeof(port_set_parameter_data) <= command::get_max_data_length(), "port_set_parameter_data exceeds maximum command data size");

        struct port_inspector_create_data
        {
            string_hash port;
        };
        static_assert(sizeof(port_inspector_create_data) <= command::get_max_data_length(), "port_inspector_create_data exceeds maximum command data size");

        struct port_inspector_destroy_data
        {
            string_hash port;
        };
        static_assert(sizeof(port_inspector_destroy_data) <= command::get_max_data_length(), "port_inspector_destroy_data exceeds maximum command data size");

        struct connection_destroy_data
        {
            string_hash connection;
        };
        static_assert(sizeof(connection_destroy_data) <= command::get_max_data_length(), "connection_destroy_data exceeds maximum command data size");

        struct connection_rename_data
        {
            string_hash connection;
            char new_name[max_name_length];
            uint64_t edited;
        };
        static_assert(sizeof(connection_rename_data) <= command::get_max_data_length(), "connection_rename_data exceeds maximum command data size");

        struct connection_port_add_data
        {
            string_hash connection;
            string_hash port;
            bool is_input;
            bool valid_chain;
            uint64_t edited;
        };
        static_assert(sizeof(connection_port_add_data) <= command::get_max_data_length(), "connection_port_add_data exceeds maximum command data size");
        
        struct connection_processor_add_data
        {
            string_hash connection;
            string_hash processor;
            bool valid_chain;
            uint64_t edited;
        };
        static_assert(sizeof(connection_processor_add_data) <= command::get_max_data_length(), "connection_processor_add_data exceeds maximum command data size");
        
        struct connection_processor_reorder_data
        {
            string_hash connection;
            string_hash processor;
            uint32_t new_index;
            bool valid_chain;
            uint64_t edited;
        };
        static_assert(sizeof(connection_processor_reorder_data) <= command::get_max_data_length(), "connection_processor_reorder_data exceeds maximum command data size");

        struct connection_property_change_data 
        {
            string_hash connection;
            uint32_t value;
            uint64_t edited;
        };
        static_assert(sizeof(connection_property_change_data ) <= command::get_max_data_length(), "connection_property_change_data  exceeds maximum command data size");

        struct config_path_data
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
        static_assert(sizeof(config_path_data) <= command::get_max_data_length(), "config_path_data exceeds maximum command data size");

        struct config_path_remove_data
        {
            uint32_t idx;
        };
        static_assert(sizeof(config_path_remove_data) <= command::get_max_data_length(), "config_path_remove_data exceeds maximum command data size");

        struct config_export_data
        {
            uint32_t path_idx;
            char filename[max_name_length];
            char name[max_name_length];
            char description[max_description_length];

            void setup(uint32_t p_idx, const char* f, const char* n, const char* d)
            {
                path_idx = p_idx;
                std::strncpy(filename, f, sizeof(filename) - 1);
                filename[sizeof(filename) - 1] = '\0';
                std::strncpy(name, n, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                std::strncpy(description, d, sizeof(description) - 1);
                description[sizeof(description) - 1] = '\0';
            }
        };
        static_assert(sizeof(config_export_data) <= command::get_max_data_length(), "config_export_data exceeds maximum command data size");

        struct config_save_data
        {
            char name[max_name_length];
            char description[max_description_length];

            void setup(const char* n, const char* d)
            {
                std::strncpy(name, n, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                std::strncpy(description, d, sizeof(description) - 1);
                description[sizeof(description) - 1] = '\0';
            }
        };
        static_assert(sizeof(config_save_data) <= command::get_max_data_length(), "config_save_data exceeds maximum command data size");

        struct config_import_data
        {
            uint32_t path_idx;
            char filename[max_name_length];

            void setup(uint32_t p_idx, const char* f)
            {
                path_idx = p_idx;
                std::strncpy(filename, f, sizeof(filename) - 1);
                filename[sizeof(filename) - 1] = '\0';
            }
        };
        static_assert(sizeof(config_import_data) <= command::get_max_data_length(), "config_import_data exceeds maximum command data size");

        struct config_info_data
        {
            uint32_t path_idx;
            char filename[max_name_length];
            char name[max_name_length];
            char description[max_description_length];
            uint64_t created;
            uint64_t modified;
            uint32_t port_count;
            uint32_t processor_count;
            uint32_t connection_count;

            void setup(uint32_t p_idx, const char* f, const char* n, const char* d, uint64_t c, uint64_t m, uint32_t po, uint32_t pr, uint32_t co)
            {
                path_idx = p_idx;
                std::strncpy(filename, f, sizeof(filename) - 1);
                filename[sizeof(filename) - 1] = '\0';
                std::strncpy(name, n, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                std::strncpy(description, d, sizeof(description) - 1);
                description[sizeof(description) - 1] = '\0';
                created = c;
                modified = m;
                port_count = po;
                processor_count = pr;
                connection_count = co;
            }
        };
        static_assert(sizeof(config_info_data) <= command::get_max_data_length(), "config_info_data exceeds maximum command data size");

        #pragma pack(pop)
    }
}