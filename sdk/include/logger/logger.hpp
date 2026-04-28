#pragma once

/**
 * @file    logger.hpp
 * @author  dexus1337
 * @brief   Defines the ADAM logger which allows to send logs to the controller from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include "controller/controller.hpp"


namespace adam 
{
    class log;

    /**
     * @class logger
     * @brief   Defines the ADAM logger which allows to send logs to the controller from external processes
     */
    class ADAM_SDK_API logger
    {
    public:

        /** @brief Constructs a new logger object.*/
        logger();

        /** @brief Destroys the logger object and cleans up resources.*/
        ~logger();

        /** @brief Establishes a connection to the main controller. */
        bool connect();

        /** @brief Disconnect and free resources. */
        bool destroy();

        /** @brief Sends a log. */
        bool log(const log& log);

        /** @brief Sends a log. */
        bool log(std::string_view txt, log::level t) { return this->log(adam::log(txt, t)); }

    protected:

        controller::queue_log m_queue_log;
    };
}