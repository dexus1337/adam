#include "module/module-internal.hpp"
#include "data/port/port-internal.hpp"
#include "data/format.hpp"
#include "controller/registry.hpp"
#include "factory/factory.hpp"

namespace adam
{
    static default_factory<port, port_internal> global_port_internal_factory = default_factory<port, port_internal>();

    module_internal::module_internal() : module("internal"_ct, adam::make_version(1, 0, 0), adam::sdk_version)
    {
        // Register internal data formats
        m_data_formats.emplace("transparent"_ct.get_hash(), &data_format_transparent);

        // Register internal port factories
        m_port_factories.emplace
        (
            port_internal::type_name(),
            registry::factory_data_port(&global_port_internal_factory, nullptr, port::direction_invalid)
        );
    }

    module_internal::~module_internal()
    {
    }
}
