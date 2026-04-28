#include "module/module.hpp"

#include "version/version.hpp"

namespace adam
{
    module::module(std::string_view name, uint32_t version)
     :  m_str_name(name),
        m_str_filepath(),
        m_mod_handle(0),
        m_ui32_req_sdk_version(adam::sdk_version),
        m_ui32_version(version),
        m_data_formats() {}

    module::~module() {}
}