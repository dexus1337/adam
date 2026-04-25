#pragma once

/**
 * @file        controller.hpp
 * @author      dexus1337
 * @brief       Defines the core ADAM controller class which manages the entire system
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class controller
     * @brief The main controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
     */
    class ADAM_SDK_API controller 
    {
    public:

        /**
         * @brief Constructs a new controller object.
         */
        controller();

        /**
         * @brief Destroys the controller object and cleans up resources.
         */
        ~controller();

    };
}