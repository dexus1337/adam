#include "module/module.hpp"

#include "version/version.hpp"

namespace adam
{
    module::module(const string_hashed& name, uint32_t version, uint32_t req_sdk_ver)
     :  m_str_name(name),
        m_str_filepath(),
        m_mod_handle(0),
        m_ui32_req_sdk_version(req_sdk_ver),
        m_ui32_version(version),
        m_data_formats(),
        m_port_factories(),
        m_filter_factories(),
        m_converter_factories()
    {

    }

    module::~module() {}
}
