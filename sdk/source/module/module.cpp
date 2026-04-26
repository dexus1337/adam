#include "module/module.hpp"

namespace adam
{
    module::module(std::string_view name, int version)
     :  m_str_name(name),
        m_str_filepath(),
        m_mod_handle(0),
        m_i_required_sdk_version(0x100),
        m_i_version(version),
        m_data_formats() {}

    module::~module() {}
}