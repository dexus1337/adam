#include "data/format.hpp"

namespace adam 
{
    data_format::data_format(const string_hashed& name, parser* parser, encoder* encoder, const module* orig_module)
        : m_str_name(name), m_parser(parser), m_encoder(encoder), m_module(orig_module) {}

    data_format::~data_format() {}
}