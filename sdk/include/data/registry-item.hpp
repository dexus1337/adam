#pragma once

/**
 * @file    registry-item.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, connections, and processors, containing a pointer to the system controller.
 * @version 1.0
 * @date    02.07.2026
 */

#include "api/api-sdk.hpp"
#include "configuration/configuration-item.hpp"

namespace adam 
{
    class controller;

    /**
     * @class registry_item
     * @brief A base class for registry items (ports, connections, processors) that holds a pointer to the controller.
     */
    class ADAM_SDK_API registry_item : public configuration_item
    {
    public:

        void set_controller(controller* ctrl) { m_controller = ctrl; }
        controller* get_controller() const { return m_controller; }

    protected:

        registry_item(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list())
            : configuration_item(item_name, default_params), m_controller(nullptr) {}

        virtual ~registry_item() = default;

        controller* m_controller = nullptr;
    };
}
