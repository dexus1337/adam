#pragma once

/**
 * @file    inspector.hpp
 * @author  dexus1337
 * @brief   Defines a class which enables to route data from ports in order to read-only inspect
 * @version 1.0
 * @date    07.05.2026
 */

 
#include "api/api.hpp"
#include "configuration/configuration-item.hpp"

namespace adam 
{
    /**
     * @class data_processor
     * @brief   Defines a class which enables to route data from ports in order to read-only inspect
     */
    class ADAM_SDK_API inspector : public configuration_item
    {
    public:

        /** @brief Destroys the data processor object and cleans up resources. */
        ~inspector();
        
    protected:

        /** @brief Constructs a new data processor object. */
        inspector(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

    };
}