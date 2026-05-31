#pragma once

/**
 * @file    configuration-parameter.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configuration parameters
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api-sdk.hpp"

#include <memory>
#include <ostream>
#include <istream>
#include <unordered_map>
#include "types/string-hashed.hpp"
#include "resources/language.hpp"

namespace adam 
{
    /**
     * @class configuration_parameter
     * @brief Defines a base class for any configuration parameters
     */
    class ADAM_SDK_API configuration_parameter 
    {

        friend class configuration_item;
        
    public:

        enum type : uint8_t
        {
            type_invalid = 0,
            type_integer,
            type_double,
            type_boolean,
            type_string,
            type_list,
            type_reference
        };

        #pragma pack(push, 1)
        struct view
        {
            type        var_type;
            string_hash name;
        };
        #pragma pack(pop)

        virtual type get_type()         const = 0;
        const string_hashed& get_name() const { return m_str_name; }
        void set_name(const string_hashed& name) { m_str_name = name; }

        const string_hashed& get_description(language lang) const;
        void set_description(language lang, const string_hashed& description);

        const std::unordered_map<language, string_hashed>& get_descriptions() const { return m_descriptions; }
        void set_descriptions(const std::unordered_map<language, string_hashed>& descriptions) { m_descriptions = descriptions; }

        /** @brief Creates a deep copy of this configuration parameter. */
        virtual std::unique_ptr<configuration_parameter> clone() const = 0;

        /** @brief Destroys the configuration_parameter object and cleans up resources. */
        virtual ~configuration_parameter();

        // --- Serialization Helpers ---

        static void serialize(std::ostream& os, const configuration_parameter* param);
        static std::unique_ptr<configuration_parameter> deserialize(std::istream& is);

        template<typename T>
        static void write_binary(std::ostream& os, const T& value) 
        {
            os.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        template<typename T>
        static void read_binary(std::istream& is, T& value) 
        {
            is.read(reinterpret_cast<char*>(&value), sizeof(T));
        }

        static void write_string(std::ostream& os, const std::string& str);
        static string_hashed read_string(std::istream& is);

    protected:

        /** @brief Constructs a new configuration_parameter object. */
        configuration_parameter();

        /** @brief Constructs a new configuration_parameter object. */
        configuration_parameter(const string_hashed& name);

        string_hashed m_str_name;     /**< The name of the configuration parameter, used for identification and lookup in the ADAM system. */
        std::unordered_map<language, string_hashed> m_descriptions;  /**< The descriptions of the parameter mapped by language, used for UI or logging. Not serialized. */
    };
}