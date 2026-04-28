#pragma once

/**
 * @file    logger-sink.hpp
 * @author  dexus1337
 * @brief   Defines a ADAM loger sink which allows to retrieve logs to the controller from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"

#include "controller/controller.hpp"


namespace adam 
{
    class log;

    /**
     * @class logger_sink
     * @brief Defines a ADAM loger sink which allows to retrieve logs to the controller from external processes
     */
    class ADAM_SDK_API logger_sink
    {
    public:

        /** @brief Constructs a new logger_sink object.*/
        logger_sink();

        /** @brief Destroys the logger_sink object and cleans up resources.*/
        ~logger_sink();

        /** @brief Establishes a connection to the main controller. */
        bool connect();

        /** @brief Disconnect and free resources. */
        bool destroy();

    protected:

        controller::queue_log_sink m_queue_log_sink;
    };
}