#pragma once

/**
 * @file    configuration-parameter-list-sorted.hpp
 * @author  dexus1337
 * @brief   Defines a sorted list of configuration parameters.
 * @version 1.0
 * @date    03.06.2026
 */

#include "api/api-sdk.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include <vector>
#include <string>

namespace adam 
{
    /**
     * @class configuration_parameter_list_sorted
     * @brief Defines a sorted list of configuration parameters.
     */
    class ADAM_SDK_API configuration_parameter_list_sorted : public configuration_parameter_list
    {
    public:
    
        configuration_parameter_list_sorted();
        configuration_parameter_list_sorted(const string_hashed& name);
        configuration_parameter_list_sorted(const configuration_parameter_list_sorted& other);
        configuration_parameter_list_sorted& operator=(const configuration_parameter_list_sorted& other);
        
        configuration_parameter_list_sorted(const configuration_parameter_list& other);
        configuration_parameter_list_sorted& operator=(const configuration_parameter_list& other);

        configuration_parameter_list_sorted(configuration_parameter_list_sorted&&) noexcept = default;
        configuration_parameter_list_sorted& operator=(configuration_parameter_list_sorted&&) noexcept = default;

        ~configuration_parameter_list_sorted() = default;

        std::unique_ptr<configuration_parameter> clone() const override 
        { 
            return std::make_unique<configuration_parameter_list_sorted>(*this); 
        }

        void add(std::unique_ptr<configuration_parameter> param) override;
        void clear() override;
        void remove(string_hash name) override;
        bool rename_child(string_hash old_name, const string_hashed& new_name) override;

        const std::vector<string_hash>& get_order() const { return m_order; }
        std::vector<string_hash>& order() { return m_order; }

    private:
        std::vector<string_hash> m_order;
    };
}
