#include "data/format.hpp"
#include "data/parser.hpp"
#include "data/encoder.hpp"

namespace adam 
{
    data_format::data_format
    (
        const string_hashed&    name,
        const factory<parser>*  parser_factory,
        const factory<encoder>* encoder_factory,
        analyzer*               analyzer,
        const module*           orig_module
    )
        : m_str_name(name),
          m_parser_factory(parser_factory),
          m_encoder_factory(encoder_factory),
          m_analyzer(analyzer),
          m_module(orig_module)
    {
    }

    data_format::~data_format() 
    {
    }

    std::unique_ptr<parser> data_format::create_parser(const string_hashed& name) const
    {
        if (!m_parser_factory)
            return nullptr;
        return m_parser_factory->create(name);
    }

    std::unique_ptr<encoder> data_format::create_encoder(const string_hashed& name) const
    {
        if (!m_encoder_factory)
            return nullptr;
        return m_encoder_factory->create(name);
    }
}