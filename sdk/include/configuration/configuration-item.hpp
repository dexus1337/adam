#pragma once

/**
 * @file    configuration-item.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configurable item in the ADAM system.
 * @version 1.0
 * @date    06.05.2026
 */

#include "api/sdk-api.hpp"
#include "types/string-hashed.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"

namespace adam 
{
    /**
     * @class configuration_item
     * @brief Defines a base class for components (like ports, filters, etc.) that manage configurable parameters.
     * 
     * Child classes should pass a list of their static default parameters to this base class constructor. 
     * The base class will instantly perform a deep clone of the defaults, making instantiation extremely fast
     * and universally smooth across the framework!
     */
    class ADAM_SDK_API configuration_item 
    {
    public:
        /** @brief Retrieves the name of this configuration item. */
        const string_hashed& get_name() const { return m_parameters.get_name(); }

        /** @brief Retrieves the cloned configuration parameters specific to this instance. */
        const configuration_parameter_list& get_parameters() const { return m_parameters; }
        configuration_parameter_list&       get_parameters()       { return m_parameters; }

        /** @brief Retrieves a specific configuration parameter by name. */
        template<typename T>
        T* get_parameter(const string_hashed& name) const { return dynamic_cast<T*>(m_parameters.get(name)); }

        /** @brief Saves the configuration item's parameters to a binary file. */
        virtual bool save(string_hashed::view filepath) const;

        /** @brief Loads the configuration item's parameters from a binary file. */
        virtual bool load(string_hashed::view filepath);

        /** @brief Adds additional parameters to this configuration item. */
        void add_parameters(const configuration_parameter_list& params);

    protected:

        /** 
         * @brief Constructs a new configuration_item and deep-copies the provided default parameters.
         * 
         * @param item_name      The unique name of this instance.
         * @param default_params The default parameter list to deeply clone into this instance.
         */
        configuration_item(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        virtual ~configuration_item();

        configuration_parameter_list m_parameters;   /**< The cloned, independent parameters for this instance. */
    };
}