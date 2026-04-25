#include "data/format/data-format.hpp"

namespace adam 
{
    data_format::data_format(const string_hashed& name, parser* parser, serializer* serializer)
        : m_str_name(name), m_parser(parser), m_serializer(serializer) {}

    data_format::data_format(std::string_view name, parser* parser, serializer* serializer)
        : m_str_name(name), m_parser(parser), m_serializer(serializer) 
    {
        m_str_name.calculate_hash();
    }

    data_format::~data_format() {}
}