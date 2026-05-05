#pragma once

/**
 * @file    configuration-parameter-list.hpp
 * @author  dexus1337
 * @brief   Defines a a list of configuration parameters, used for defining multiple related configuration parameters in a structured way.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"

#include <string_view>
#include <unordered_map>
#include <memory>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_list
     * @brief Defines a a list of configuration parameters, used for defining multiple related configuration parameters in a structured way.
     */
    class ADAM_SDK_API configuration_parameter_list : public configuration_parameter
    {
    public:
    
        /** @brief Constructs a new configuration_parameter_list object. */
        configuration_parameter_list(const string_hashed& name);
        configuration_parameter_list(std::string_view name);

        /** @brief Destroys the configuration_parameter_list object and cleans up resources. */
        ~configuration_parameter_list();

        type get_type() const override { return list; }

        /** @brief Adds a child parameter to the list. */
        void add(std::unique_ptr<configuration_parameter> param);

        /** @brief Retrieves the list of child parameters. */
        const std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter>>& get_children() const { return m_children; }

        /** @brief Retrieves a child parameter by its name. Returns nullptr if not found. */
        configuration_parameter* get(const string_hashed& name) const;

    private:

        std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter>> m_children;
    };
}