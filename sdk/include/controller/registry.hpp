#pragma once

/**
 * @file    registry.hpp
 * @author  dexus1337
 * @brief   Defines a registry for managing configuration items, including loading from and saving to a fast, binary file format.
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"

#include <string_view>
#include <memory>

#include "configuration/parameters/configuration-parameter-list.hpp"


namespace adam
{
    class controller;
    class port_input;
    class port_output;
    class converter;
    class filter;

    /**
     * @class registry
     * @brief Manages the global application configuration, including loading from and saving to a fast, binary file format.
     */
    class ADAM_SDK_API registry
    {
        friend class controller;

    public:
        /** @brief Retrieves the list of configuration parameters for input ports. */
        std::unordered_map<string_hashed, std::unique_ptr<port_input>>&     input_ports()   { return m_input_ports; }
        std::unordered_map<string_hashed, std::unique_ptr<port_output>>&    output_ports()  { return m_output_ports; }
        std::unordered_map<string_hashed, std::unique_ptr<filter>>&         filters()       { return m_filters; }
        std::unordered_map<string_hashed, std::unique_ptr<converter>>&      converters()    { return m_converters; }

        /** @brief Saves the entire configuration tree to a binary file. */
        bool save(std::string_view filepath) const;

        /** @brief Loads the entire configuration tree from a binary file. */
        bool load(std::string_view filepath);

    protected:

        /** @brief Constructs a new registry object. */
        registry(const controller* ctrl);

        /** @brief Destroys the registry object. */
        ~registry();

        configuration_parameter_list m_general;                                         /**< General configuration parameters. */
        
        std::unordered_map<string_hashed, std::unique_ptr<port_input>> m_input_ports;   /**< The list of configuration parameters for input ports. */
        std::unordered_map<string_hashed, std::unique_ptr<port_output>> m_output_ports; /**< The list of configuration parameters for output ports. */
        std::unordered_map<string_hashed, std::unique_ptr<filter>> m_filters;           /**< The list of configuration parameters for filters. */
        std::unordered_map<string_hashed, std::unique_ptr<converter>> m_converters;     /**< The list of configuration parameters for converters. */

        const controller* m_controller;                                                 /**< A pointer to the controller, used for accessing shared resources and orchestrating interactions between components. */
    };
}