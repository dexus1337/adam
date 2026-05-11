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

#include "configuration/configuration-item.hpp"


namespace adam
{
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
        /** @brief Retrieves the list of configuration parameters for input ports. */
        std::unordered_map<string_hashed, std::unique_ptr<port>>&       ports()         { return m_ports; }
        std::unordered_map<string_hashed, std::unique_ptr<filter>>&     filters()       { return m_filters; }
        std::unordered_map<string_hashed, std::unique_ptr<converter>>&  converters()    { return m_converters; }
        std::unordered_map<string_hashed, std::unique_ptr<connection>>& connections()   { return m_connections; }

        /** @brief Saves the entire configuration tree to a binary file. */
        bool save(string_hashed::view filepath) const override;

        /** @brief Loads the entire configuration tree from a binary file. */
        bool load(string_hashed::view filepath) override;

        /** @brief Clears all configuration items and parameters */
        void clear();

    protected:

        /** @brief Constructs a new registry object. */
        registry(const controller& ctrl);

        /** @brief Destroys the registry object. */
        ~registry();

        std::unordered_map<string_hashed, std::unique_ptr<port>>        m_ports;        /**< The list of configuration parameters for ports. */
        std::unordered_map<string_hashed, std::unique_ptr<filter>>      m_filters;      /**< The list of configuration parameters for filters. */
        std::unordered_map<string_hashed, std::unique_ptr<converter>>   m_converters;   /**< The list of configuration parameters for converters. */
        std::unordered_map<string_hashed, std::unique_ptr<connection>>  m_connections;  /**< The list of configuration parameters for connections. */

        const controller& m_controller;                                                 /**< A reference to the controller, used for accessing shared resources and orchestrating interactions between components. */
    };
}