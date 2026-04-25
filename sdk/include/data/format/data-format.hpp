#pragma once

/**
 * @file        data-format.hpp
 * @author      dexus1337
 * @brief       Defines a data format base class for any data formats used in the ADAM system, providing a common interface for handling different types of data.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"
#include "string/string-hashed.hpp"


namespace adam 
{
    class parser;
    class serializer;

    /**
     * @class data_format
     * @brief A base class for any data formats used in the ADAM system, providing a common interface for handling different types of data.
     */
    class ADAM_SDK_API data_format 
    {
    public:

        /** @brief Constructs a new data_format object. */
        data_format(const string_hashed& name, parser* parser = nullptr, serializer* serializer = nullptr);

        /** @brief Constructs a new data_format object. */
        data_format(std::string_view name, parser* parser = nullptr, serializer* serializer = nullptr);

        /** @brief Destroys the data_format object and cleans up resources. */
        ~data_format();

        string_hashed   get_name()          const { return m_str_name; }
        parser*         get_parser()        const { return m_parser; }
        serializer*     get_serializer()    const { return m_serializer; } 

    protected:

        string_hashed   m_str_name;     /**< The name of the data format, used for identification and lookup in the ADAM system. */
        parser*         m_parser;       /**< A pointer to the parser associated with this data format, responsible for parsing data in this format. */
        serializer*     m_serializer;   /**< A pointer to the serializer associated with this data format, responsible for serializing data in this format. */
    };

    static const data_format data_format_transparent = data_format( "transparent" );   /**< A predefined data format representing raw, unprocessed data that can be passed through the system without any parsing or serialization. */
}