#pragma once

/**
 * @file        response.hpp
 * @author      dexus1337
 * @brief       Defines a response for the controller
 * @version     1.0
 * @date        27.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class response
     * @brief Defines a response for the controller
     */
    class ADAM_SDK_API response 
    {
    public:

        enum type
        {
            invalid = 0,
            success,
            failed
        };

        /** @brief Constructs a new response object.*/
        response(type t = invalid) : m_type(t) {}

        /** @brief Destroys the response object and cleans up resources.*/
        ~response() = default;

        type get_type() const { return m_type; }

    protected:

        type m_type;

    };
}