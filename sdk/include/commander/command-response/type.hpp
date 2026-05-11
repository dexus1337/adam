#pragma once

/**
 * @file    type.hpp
 * @author  dexus1337
 * @brief   Defines a template class for commands and responses, which can hold any data up to a certain size.
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
     * @class command_response_type
     * @brief Defines a template class for commands and responses, which can hold any data up to a certain size.
     */
    #pragma pack(push, 1) // align to 1 byte
    template<typename type>
    class ADAM_SDK_API command_response_type 
    {
    public:

        static_assert(sizeof(type) == 1, "command_response_type template type must be exactly 1 byte in size");

        static constexpr size_t size_in_bytes = 512;

        /** @brief Constructs a new command_response_type object.*/
        command_response_type(type t = static_cast<type>(0)) : m_type(t), m_b_extended(false) {}

        /** @brief Destroys the command_response_type object and cleans up resources.*/
        ~command_response_type() = default;

        type get_type()             const { return m_type; }
        const uint8_t* get_data()   const { return m_data; }
        bool is_extended()          const { return m_b_extended; }

        template<typename T>
        const T* get_data_as()      const { return reinterpret_cast<const T*>(m_data); }

        uint8_t* data() { return m_data; }

        template<typename T>
        T* data_as() { return reinterpret_cast<T*>(m_data); }

        void set_extended(bool extended = true) { m_b_extended = extended; }

    protected:

        type m_type;
        bool m_b_extended; /**< If extended, the following command_response_type is pure data*/

        static constexpr size_t max_data_length = size_in_bytes - sizeof(m_type) - sizeof(m_b_extended); /**< Ensure an command_response_type is exactly the wanted size in bytes*/

        uint8_t m_data[max_data_length];
    };
    #pragma pack(pop)
}