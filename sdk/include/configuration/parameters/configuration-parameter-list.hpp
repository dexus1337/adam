#pragma once

/**
 * @file    configuration-parameter-list.hpp
 * @author  dexus1337
 * @brief   Defines a a list of configuration parameters, used for defining multiple related configuration parameters in a structured way.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/sdk-api.hpp"

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
        configuration_parameter_list();

        /** @brief Constructs a new configuration_parameter_list object. */
        configuration_parameter_list(const string_hashed& name);

        /** @brief Copy constructor. Performs a deep copy of all child parameters. */
        configuration_parameter_list(const configuration_parameter_list& other);

        /** @brief Copy assignment operator. Performs a deep copy of all child parameters. */
        configuration_parameter_list& operator=(const configuration_parameter_list& other);

        /** @brief Move semantics. */
        configuration_parameter_list(configuration_parameter_list&&) noexcept = default;
        configuration_parameter_list& operator=(configuration_parameter_list&&) noexcept = default;

        /** @brief Destroys the configuration_parameter_list object and cleans up resources. */
        ~configuration_parameter_list();

        type get_type() const override { return list; }

        /** @brief Creates a deep copy of this configuration parameter list. */
        std::unique_ptr<configuration_parameter> clone() const override { return std::make_unique<configuration_parameter_list>(*this); }

        /** @brief Adds a child parameter to the list. */
        void add(std::unique_ptr<configuration_parameter> param);

        /** @brief Retrieves the list of child parameters. */
        const std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter>>& get_children() const { return m_children; }

        /** @brief Retrieves the mutable list of child parameters. */
        std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter>>& children() { return m_children; }

        /** @brief Clears all child parameters. */
        void clear();

        /** @brief Retrieves a child parameter by its name. Returns nullptr if not found. */
        configuration_parameter* get(const string_hashed& name) const;

    private:

        std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter>> m_children;
    };
}