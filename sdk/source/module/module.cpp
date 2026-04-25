#include "module/module.hpp"

namespace adam
{
    module::module(std::string_view name) 
     :  m_str_name(name), 
        m_i_require_sdk_version(0x100),
        m_data_formats() {}

    module::~module() {}
}