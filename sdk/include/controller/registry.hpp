#pragma once

/**
 * @file    registry.hpp
 * @author  dexus1337
 * @brief   Defines a registry for managing configuration items, including loading from and saving to a fast, binary file format.
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api-sdk.hpp"

#include <string_view>
#include <memory>
#include <unordered_map>

#include "configuration/configuration-item.hpp"
#include "factory/factory.hpp"
#include "resources/language.hpp"
#include "controller/registry-module-manager.hpp"
#include "data/port.hpp"
#include "data/connection.hpp"
#include "data/processor.hpp"
#include "types/string-hashed.hpp"

namespace adam
{
    class data_format;
    class controller;
    class processor;
    class connection;
    class module_internal;

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

        using data_format_map               = std::unordered_map<string_hash, const data_format*>;                              /**< A type alias for a map of data formats supported by a module, indexed by their hashed string names for efficient lookup. */
        
        using port_map                      = std::unordered_map<string_hash, std::unique_ptr<port>>;                           /**< A map for storing port instances. */
        using processor_map                 = std::unordered_map<string_hash, std::unique_ptr<processor>>;                      /**< A map for storing processor instances. */
        using connection_map                = std::unordered_map<string_hash, std::unique_ptr<connection>>;                     /**< A map for storing connection instances. */
        using unavailable_port_map          = std::unordered_map<string_hash, std::unique_ptr<port::unavailable_info>>;         /**< A map for storing unavailable port information. */
        using unavailable_connection_map    = std::unordered_map<string_hash, std::unique_ptr<connection::unavailable_info>>;   /**< A map for storing unavailable connection information. */
        using unavailable_processor_map     = std::unordered_map<string_hash, std::unique_ptr<processor::unavailable_info>>;    /**< A map for storing unavailable processor information. */

        template<typename T>
        struct factory_data
        {
            factory_data() = default;
            factory_data(factory<T>* ptr) : factory_ptr(ptr), parameters(nullptr) {}
            factory_data(factory<T>* ptr, const configuration_parameter_list* params) : factory_ptr(ptr), parameters(params) {}

            factory<T>*                         factory_ptr = nullptr;
            const configuration_parameter_list* parameters = nullptr;
        };

        struct factory_data_port : public factory_data<port>
        {
            factory_data_port() = default;
            factory_data_port(factory<port>* ptr) : factory_data<port>(ptr), direction(port::direction_invalid) {}
            factory_data_port(factory<port>* ptr, const configuration_parameter_list* params, port::direction dir)  : factory_data<port>(ptr, params), direction(dir) {}

            port::direction direction = port::direction_invalid;
        }; 

        struct factory_data_processor : public factory_data<processor>
        {
            factory_data_processor() = default;
            factory_data_processor(factory<processor>* ptr, const configuration_parameter_list* params, string_hash idt = 0, string_hash idtm = 0, string_hash odt = 0, string_hash odtm = 0) 
                : factory_data<processor>(ptr, params), input_datatype(idt), input_datatype_module(idtm), output_datatype(odt), output_datatype_module(odtm) {}

            string_hash input_datatype;
            string_hash input_datatype_module;
            string_hash output_datatype;
            string_hash output_datatype_module;
        }; 

        using port_factory_map              = std::unordered_map<string_hashed, factory_data_port>;         /**< A map of factories for creating ports provided by a module. */
        using processor_factory_map         = std::unordered_map<string_hashed, factory_data_processor>;    /**< A map of factories for creating processors provided by a module. */

        /** @brief Explicitly delete copy semantics to prevent dllexport from generating implicit copies of unique_ptr maps. */
        registry(const registry&) = delete;
        registry& operator=(const registry&) = delete;

        const port_map&                     get_ports()                     const   { return m_ports; }
        const processor_map&                get_processors()                const   { return m_processors; }
        const connection_map&               get_connections()               const   { return m_connections; }
        const unavailable_port_map&         get_unavailable_ports()         const   { return m_unavailable_ports; }
        const unavailable_connection_map&   get_unavailable_connections()   const   { return m_unavailable_connections; }
        const unavailable_processor_map&    get_unavailable_processors()    const   { return m_unavailable_processors; }
        const registry_module_manager&      get_modules()                   const   { return m_modules; }

        port_map&                           ports()                     { return m_ports; }
        processor_map&                      processors()                { return m_processors; }
        connection_map&                     connections()               { return m_connections; }
        unavailable_port_map&               unavailable_ports()         { return m_unavailable_ports; }
        unavailable_connection_map&         unavailable_connections()   { return m_unavailable_connections; }
        unavailable_processor_map&          unavailable_processors()    { return m_unavailable_processors; }
        registry_module_manager&            modules()                   { return m_modules; }
        
        /** @brief Retrieves the list of configured module paths. */
        const configuration_parameter_list* get_module_paths() const;

        /** @brief Retrieves a pointer to a loaded module (either internal or external) by its hashed name. */
        const module* get_module(string_hash module_hash) const;

        /** @brief Retrieves a pointer to a data format (either from internal or external module) by its hashed name and originating module. */
        const data_format* get_data_format(string_hash fmt_hash, string_hash mod_hash) const;

        /** @brief Creates a new port using the appropriate factory and adds it to the registry. Returns the status of the operation. */
        status create_port(const string_hashed& name, string_hash type, string_hash type_module, port** out_port = nullptr);

        /** @brief Destroys a port from the registry by its hash, and cleans up its connections. */
        status destroy_port(string_hash hash);

        /** @brief Renames a port safely. */
        status rename_port(string_hash hash, const string_hashed& new_name);

        /** @brief Creates a new processor using the appropriate factory and adds it to the registry. Returns the status of the operation. */
        status create_processor(const string_hashed& name, string_hash type, string_hash type_module, bool is_filter = false, processor** out_processor = nullptr);

        /** @brief Destroys a processor from the registry by its hash, and cleans up its connections. */
        status destroy_processor(string_hash hash);

        /** @brief Renames a processor safely. */
        status rename_processor(string_hash hash, const string_hashed& new_name);

        /** @brief Creates a new connection and adds it to the registry. Returns the status of the operation. */
        status create_connection(const string_hashed& name, connection** out_connection = nullptr);

        /** @brief Destroys a connection from the registry by its hash. */
        status destroy_connection(string_hash hash);

        /** @brief Renames a connection safely. */
        status rename_connection(string_hash hash, const string_hashed& new_name);

        /** @brief Adds a port to an existing connection. */
        status connection_add_port(string_hash conn_hash, string_hash port_hash, bool is_input);

        /** @brief Removes a port from an existing connection. */
        status connection_remove_port(string_hash conn_hash, string_hash port_hash, bool is_input);

        /** @brief Adds a processor to an existing connection. */
        status connection_add_processor(string_hash conn_hash, string_hash processor_hash);

        /** @brief Removes a processor from an existing connection. */
        status connection_remove_processor(string_hash conn_hash, string_hash processor_hash);

        /** @brief Reorders a processor in an existing connection. */
        status connection_reorder_processor(string_hash conn_hash, string_hash processor_hash, uint32_t new_index);

        /** @brief Saves the entire configuration tree to a binary file. */
        bool save(string_hashed::view filepath) const override;

        /** @brief Loads the entire configuration tree from a binary file. */
        bool load(string_hashed::view filepath) override;

        /** @brief Clears all configuration items and parameters */
        void clear();

        /** @brief Registers a statically compiled internal module in the registry. */
        void register_internal_module(const module* mod);

        /** @brief Resumes active items based on their loaded configuration, must be called after system initialization */
        void resume_active_items();

        /** @brief Stops active items, must be called before system shutdown */
        void stop_items();

        /** @brief Adds a new module path to the configuration if it doesn't already exist. */
        bool add_module_path(const string_hashed& path, uint32_t* index = nullptr);

        /** @brief Removes a module path from the configuration. */
        bool remove_module_path(uint32_t index);

        /** @brief Tries to create and restore any unavailable ports that belong to the newly loaded module. */
        void retry_unavailable_ports(string_hash module_hash);

        /** @brief Marks ports originating from the given module as unavailable. */
        void mark_ports_unavailable(string_hash module_hash);

        /** @brief Tries to create and restore any unavailable processors that belong to the newly loaded module. */
        void retry_unavailable_processors(string_hash module_hash);

        /** @brief Marks processors originating from the given module as unavailable. */
        void mark_processors_unavailable(string_hash module_hash);

        /** @brief Tries to create and restore any unavailable connections that belong to the newly loaded module. */
        void retry_unavailable_connections(string_hash module_hash);

        /** @brief Marks connections originating from the given module as unavailable. */
        void mark_connections_unavailable(string_hash module_hash);

        /** @brief Deep copies a list of configuration parameters. */
        static void copy_parameters(configuration_parameter_list* target, configuration_parameter_list* source);

    protected:

        /** @brief Constructs a new registry object. */
        registry(controller& ctrl);

        /** @brief Destroys the registry object. */
        ~registry();

        port_map                    m_ports;                    /**< The list of configuration parameters for ports. */
        processor_map               m_processors;               /**< The list of configuration parameters for processors. */
        connection_map              m_connections;              /**< The list of configuration parameters for connections. */

        unavailable_port_map        m_unavailable_ports;        /**< The list of ports that failed to load because their module was missing. */
        unavailable_connection_map  m_unavailable_connections;  /**< The list of connections that failed to load because their module format was missing. */
        unavailable_processor_map   m_unavailable_processors;   /**< The list of processors that failed to load because their module was missing. */

        controller&                 m_controller;               /**< A reference to the controller, used for accessing shared resources and orchestrating interactions between components. */
        registry_module_manager     m_modules;                  /**< Manages external modules loaded into the registry. */
    };
}