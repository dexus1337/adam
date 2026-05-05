#pragma once

/**
 * @file    configuration-item.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configuration objects
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"


#include "configuration/parameters/configuration-parameter-list.hpp"

namespace adam 
{
    /**
     * @class configuration_item
     * @brief Defines a base class for any configuration objects
     */
    class ADAM_SDK_API configuration_item 
    {
    public:

        /** @brief Retrieves the list of configuration parameters associated with this configuration item. */
        const configuration_parameter_list& get_parameters() const { return m_parameters; }

    protected:

        /** @brief Constructs a new configuration_item object. */
        configuration_item();

        /** @brief Destroys the configuration_item object and cleans up resources. */
        ~configuration_item();

        configuration_parameter_list m_parameters; /**< The list of configuration parameters associated with this configuration item. */
    };
}