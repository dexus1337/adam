#include "data/format.hpp"

namespace adam 
{
    data_format::data_format(const string_hashed& name, parser* parser, serializer* serializer, const module* orig_module)
        : m_str_name(name), m_parser(parser), m_serializer(serializer), m_module(orig_module) {}

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