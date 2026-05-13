#pragma once

/**
 * @file    message_type.hpp
 * @author  dexus1337
 * @brief   Defines a template class for messages in the commander, which can hold any data up to a certain size.
 * @version 1.0
 * @date    27.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <cstdint>
#include <cstddef>

#include "types/string-hashed.hpp"

namespace adam 
{
    /**
     * @class commander_message_template
     * @brief Defines a template class for messages in the commander, which can hold any data up to a certain size.
     */
    #pragma pack(push, 1) // align to 1 byte
    template<typename message_type>
    class ADAM_SDK_API commander_message_template 
    {
    public:

        static_assert(sizeof(message_type) == 2, "commander_message_template template message_type must be exactly 2 bytes in size");

        static constexpr size_t size_in_bytes = 512;

        static constexpr size_t get_max_data_length() { return max_data_length; }

        /** @brief Constructs a new commander_message_template object.*/
        commander_message_template(message_type t = static_cast<message_type>(0)) : m_type(t), m_b_extended(false) {}

        /** @brief Destroys the commander_message_template object and cleans up resources.*/
        ~commander_message_template() = default;

        message_type get_type()     const { return m_type; }
        const uint8_t* get_data()   const { return m_data; }
        bool is_extended()          const { return m_b_extended; }

        template<typename T>
        const T* get_data_as()      const { return reinterpret_cast<const T*>(m_data); }

        message_type& type()        { return m_type; }
        uint8_t* data()             { return m_data; }

        template<typename T>
        T* data_as() { return reinterpret_cast<T*>(m_data); }

        void set_extended(bool extended = true) { m_b_extended = extended; }

    protected:

        message_type m_type;
        bool m_b_extended; /**< If extended, the following commander_message_template is pure data*/

        static constexpr size_t max_data_length = size_in_bytes - sizeof(m_type) - sizeof(m_b_extended); /**< Ensure an commander_message_template is exactly the wanted size in bytes*/

        uint8_t m_data[max_data_length];
    };
    #pragma pack(pop)
}