#pragma once

/**
 * @file    command.hpp
 * @author  dexus1337
 * @brief   Defines a command for the controller
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class command
     * @brief Defines a command for the controller
     */
    class ADAM_SDK_API command 
    {
    public:

        enum type
        {
            invalid = 0,
            login,
            logout
        };

        /** @brief Constructs a new command object.*/
        command(type t = invalid) : m_type(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;

        type get_type() const { return m_type; }

    protected:

        type m_type;

    };
}