#pragma once

/**
 * @file    registry.hpp
 * @author  dexus1337
 * @brief   Defines a registry for managing configuration items, including loading from and saving to a fast, binary file format.
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <string_view>
#include <memory>
#include <unordered_map>

#include "configuration/configuration-item.hpp"
#include "factory/factory.hpp"
#include "resources/language.hpp"
#include "controller/registry-module-manager.hpp"

namespace adam
{
    class data_format;
    class controller;
    class port;
    class converter;
    class filter;
    class connection;

    /**
     * @class registry
     * @brief Manages the global application configuration, including loading from and saving to a fast, binary file format.
     */
    class ADAM_SDK_API registry : public configuration_item
    {
        friend class controller;

    public:
    
        enum status : uint16_t
        {
            status_success = 0,
            status_error_port_already_exists,
            status_error_module_not_found,
            status_error_factory_not_found,
            status_error_port_not_found,
            status_error_creation_failed,
            status_error_connection_already_exists,
            status_error_connection_not_found
        };

        static std::string_view get_status_text(status status, language lang);

        /** @brief Retrieves the default configuration parameters for ports. */
        static const configuration_parameter_list& get_default_parameters();

        using data_format_map       = std::unordered_map<string_hashed::hash_datatype, const data_format*>;            /**< A type alias for a map of data formats supported by a module, indexed by their hashed string names for efficient lookup. */
        
        using port_map              = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<port>>;         /**< A map for storing port instances. */
        using filter_map            = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<filter>>;       /**< A map for storing filter instances. */
        using converter_map         = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<converter>>;    /**< A map for storing converter instances. */
        using connection_map        = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<connection>>;   /**< A map for storing connection instances. */

        using port_factory_map      = std::unordered_map<string_hashed::hash_datatype, const factory<port>*>;          /**< A map of factories for creating ports provided by a module. */
        using filter_factory_map    = std::unordered_map<string_hashed::hash_datatype, const factory<filter>*>;        /**< A map of factories for creating filters provided by a module. */
        using converter_factory_map = std::unordered_map<string_hashed::hash_datatype, const factory<converter>*>;     /**< A map of factories for creating converters provided by a module. */

        /** @brief Explicitly delete copy semantics to prevent dllexport from generating implicit copies of unique_ptr maps. */
        registry(const registry&) = delete;
        registry& operator=(const registry&) = delete;

        /** @brief Retrieves the list of configuration parameters for input ports. */
        port_map&       ports()         { return m_ports; }
        filter_map&     filters()       { return m_filters; }
        converter_map&  converters()    { return m_converters; }
        connection_map& connections()   { return m_connections; }

        /** @brief Retrieves the module manager. */
        registry_module_manager&       modules()       { return m_modules; }
        const registry_module_manager& get_modules() const { return m_modules; }

        /** @brief Creates a new port using the appropriate factory and adds it to the registry. Returns the status of the operation. */
        status create_port(const string_hashed& name, const string_hashed& type, const string_hashed& module_name = string_hashed(), port** out_port = nullptr);

        /** @brief Destroys a port from the registry by its hash, and cleans up its connections. */
        status destroy_port(string_hashed::hash_datatype hash);

        /** @brief Creates a new connection and adds it to the registry. Returns the status of the operation. */
        status create_connection(const string_hashed& name, connection** out_connection = nullptr);

        /** @brief Destroys a connection from the registry by its hash. */
        status destroy_connection(string_hashed::hash_datatype hash);

        /** @brief Saves the entire configuration tree to a binary file. */
        bool save(string_hashed::view filepath) const override;

        /** @brief Loads the entire configuration tree from a binary file. */
        bool load(string_hashed::view filepath) override;

        /** @brief Clears all configuration items and parameters */
        void clear();

        /** @brief Retrieves the list of configured module paths. */
        configuration_parameter_list* get_module_paths() const;

        /** @brief Adds a new module path to the configuration if it doesn't already exist. */
        bool add_module_path(const string_hashed& path);

        /** @brief Removes a module path from the configuration. */
        bool remove_module_path(const string_hashed& path);

    protected:

        /** @brief Constructs a new registry object. */
        registry(controller& ctrl);

        /** @brief Destroys the registry object. */
        ~registry();

        port_map            m_ports;                /**< The list of configuration parameters for ports. */
        filter_map          m_filters;              /**< The list of configuration parameters for filters. */
        converter_map       m_converters;           /**< The list of configuration parameters for converters. */
        connection_map      m_connections;          /**< The list of configuration parameters for connections. */

        port_factory_map    m_default_port_factory; /**< A map of default factories for creating ports, used when loading configurations that reference ports without specific factory information. */

        controller&             m_controller;       /**< A reference to the controller, used for accessing shared resources and orchestrating interactions between components. */
        registry_module_manager m_modules;          /**< Manages external modules loaded into the registry. */
    };
}