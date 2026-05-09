#pragma once

/**
 * @file    command.hpp
 * @author  dexus1337
 * @brief   Defines a command for the controller
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/api.hpp"

#include <cstdint>
#include <cstddef>

#include "types/string-hashed.hpp"

namespace adam 
{
    /**
     * @class command
     * @brief Defines a command for the controller
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API command 
    {
    public:

        static constexpr size_t command_size_in_bytes = 512;

        enum type : uint8_t
        {
            invalid = 0,
            login,
            logout,
            set_language,

            inspector_create,
            inspector_destroy
        };

        /** @brief Constructs a new command object.*/
        command(type t = invalid) : m_type(t) {}

        /** @brief Destroys the command object and cleans up resources.*/
        ~command() = default;

        type get_type()             const { return m_type; }

        const uint8_t* get_data()   const { return m_data; }

        template<typename T>
        const T* get_data_as()      const { return reinterpret_cast<const T*>(m_data); }

        struct inspector_create_data
        {
            string_hashed::hash_datatype port;
        };

        struct inspector_destroy_data
        {
            string_hashed::hash_datatype port;
        };

    protected:

        type m_type;

        static constexpr size_t max_data_length = command_size_in_bytes - sizeof(m_type); /**< Ensure an command is exactly the wanted size in bytes*/

        uint8_t m_data[max_data_length]; 
    };
    #pragma pack(pop)

    static_assert(sizeof(command) == command::command_size_in_bytes, "command size mismatch");
}