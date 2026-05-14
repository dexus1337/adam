#pragma once

/**
 * @file    response.hpp
 * @author  dexus1337
 * @brief   Defines a response for the controller
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <cstddef>
#include <cstddef>

#include "type.hpp"


namespace adam 
{

    enum class response_status : uint16_t
    {
        invalid = 0,
        success,
        unknown,
        failed,
        denied,
        command_send_failed,
        response_receive_failed,
        inspector_already_exists,
        inspector_creation_failed,
        inspector_start_failed,
        inspector_not_found
    };

    /**
     * @class response
     * @brief Defines a response for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API response : public commander_message_template<response_status>
    {
    public:

        /** @brief Constructs a new response object.*/
        response(response_status t = response_status::invalid) : commander_message_template(t) {}

        /** @brief Destroys the response object and cleans up resources.*/
        ~response() = default;
    };
    #pragma pack(pop)
}