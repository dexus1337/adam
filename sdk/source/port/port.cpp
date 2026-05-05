#include "port/port.hpp"


#include "data/format/data-format.hpp"


namespace adam 
{
    port::port() : m_data_format(&data_format_transparent) {}

    port::~port() {}
}