#pragma once

/**
 * @file    registry-configuration-manager.hpp
 * @author  Antigravity
 * @brief   Defines a manager for handling portable configurations within the ADAM system.
 */

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace adam 
{
    class controller;
    class registry;

    /**
     * @class   registry_configuration_manager
     * @brief   Manages the discovery, loading, and saving of portable configurations in the ADAM system.
     */
    class ADAM_SDK_API registry_configuration_manager
    {
    public:
        struct config_file_info
        {
            uint32_t path_idx;
            std::string filename;
            std::string name;
            std::string description;
            uint64_t created;
            uint64_t modified;
            uint32_t port_count;
            uint32_t processor_count;
            uint32_t connection_count;
        };

        registry_configuration_manager(controller& ctrl);
        ~registry_configuration_manager();

        const configuration_parameter_list* get_config_paths() const;
        bool add_config_path(const string_hashed& path, uint32_t* index = nullptr);
        bool remove_config_path(uint32_t index);

        std::vector<config_file_info> scan_for_configs() const;

        bool load_config(uint32_t path_idx, const std::string& filename);
        bool save_config(uint32_t path_idx, const std::string& filename, const std::string& name, const std::string& description);

        std::string get_config_name() const { return m_config_name; }
        std::string get_config_description() const { return m_config_description; }
        uint64_t get_config_created() const { return m_config_created; }
        uint64_t get_config_modified() const { return m_config_modified; }

        void set_metadata(const std::string& name, const std::string& description, uint64_t created, uint64_t modified)
        {
            m_config_name = name;
            m_config_description = description;
            m_config_created = created;
            m_config_modified = modified;
        }

        void write_header(std::ostream& os) const;
        bool read_header(std::istream& is);

    private:
        controller& m_controller;
        std::string m_config_name;
        std::string m_config_description;
        uint64_t    m_config_created;
        uint64_t    m_config_modified;
    };
}
