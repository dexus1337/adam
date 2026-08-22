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
#include "factory/factory.hpp"

namespace adam 
{
    class module;
    class parser;
    class encoder;
    class analyzer;

    /**
     * @class data_format
     * @brief A base class for any data formats used in the ADAM system, providing a common interface for handling different types of data.
     */
    class ADAM_SDK_API data_format 
    {
    public:

        data_format
        (
            const string_hashed&    name,
            const factory<parser>*  parser_factory = nullptr,
            const factory<encoder>* encoder_factory = nullptr,
            analyzer*               analyzer = nullptr,
            const module*           orig_module = nullptr
        );
        ~data_format();

        data_format(const data_format&)            = delete;
        data_format& operator=(const data_format&) = delete;
        data_format(data_format&&)                 = delete;
        data_format& operator=(data_format&&)      = delete;

        bool operator==(const data_format& other) const { return m_str_name == other.m_str_name; }
        bool operator!=(const data_format& other) const { return m_str_name != other.m_str_name; }

        const string_hashed&    get_name()            const { return m_str_name; }
        const factory<parser>*  get_parser_factory()  const { return m_parser_factory; }
        const factory<encoder>* get_encoder_factory() const { return m_encoder_factory; }
        analyzer*               get_analyzer()        const { return m_analyzer; }
        const module*           get_origin_module()   const { return m_module; }
        void                    set_origin_module(const module* mod) { m_module = mod; }

        bool has_parser()  const { return m_parser_factory != nullptr; }
        bool has_encoder() const { return m_encoder_factory != nullptr; }

        std::unique_ptr<parser>  create_parser(const string_hashed& name = "parser"_ct) const;
        std::unique_ptr<encoder> create_encoder(const string_hashed& name = "encoder"_ct) const;

    protected:

        string_hashed           m_str_name;
        const factory<parser>*  m_parser_factory;
        const factory<encoder>* m_encoder_factory;
        analyzer*               m_analyzer;
        const module*           m_module;
    };
}