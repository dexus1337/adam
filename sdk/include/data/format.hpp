#pragma once

/**
 * @file    format.hpp
 * @author  dexus1337
 * @brief   Defines a data format base class for any data formats used in the ADAM system, providing a common interface for handling different types of data.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"


namespace adam 
{
    class module;
    class parser;
    class encoder;

    /**
     * @class data_format
     * @brief A base class for any data formats used in the ADAM system, providing a common interface for handling different types of data.
     */
    class ADAM_SDK_API data_format 
    {
    public:

        data_format(const string_hashed& name, parser* parser = nullptr, encoder* encoder = nullptr, const module* orig_module = nullptr);
        ~data_format();

        const string_hashed&    get_name()          const { return m_str_name; }
        parser*                 get_parser()        const { return m_parser; }
        encoder*                get_encoder()       const { return m_encoder; } 
        const module*           get_origin_module() const { return m_module; }
        void                    set_origin_module(const module* mod) { m_module = mod; }

        data_format(const data_format&)               = delete;
        data_format& operator=(const data_format&)    = delete;
        data_format(data_format&&)                    = delete;
        data_format& operator=(data_format&&)         = delete;

        bool operator==(const data_format& other) const { return &other == this; }
        bool operator!=(const data_format& other) const { return &other != this; }

    protected:

        string_hashed   m_str_name;     /**< The name of the data format, used for identification and lookup in the ADAM system. */
        parser*         m_parser;       /**< A pointer to the parser associated with this data format, responsible for parsing data in this format. */
        encoder*        m_encoder;      /**< A pointer to the encoder associated with this data format, responsible for encoding data in this format. */
        const module*   m_module;       /**< The origin this dataformat comes from */
    };
}