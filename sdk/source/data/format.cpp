#include "data/format.hpp"

namespace adam 
{
    data_format::data_format(const string_hashed& name, parser* parser, serializer* serializer)
        : m_str_name(name), m_parser(parser), m_serializer(serializer) {}

    data_format::data_format(string_hashed::view name, parser* parser, serializer* serializer)
        : m_str_name(name), m_parser(parser), m_serializer(serializer) 
    {
        m_str_name.calculate_hash();
    }

    data_format::data_format(const data_format& df) 
        : m_str_name(df.m_str_name), m_parser(df.m_parser), m_serializer(df.m_serializer) {}

    data_format::~data_format() {}

    data_format& data_format::operator=(const data_format& df)
    {
        if (this != &df) 
        {
            m_str_name = df.m_str_name;
            m_parser = df.m_parser;
            m_serializer = df.m_serializer;
        }

        return *this;
    }

}