#include "module/internals/essential/module-essential.hpp"

#include "data/format.hpp"
#include "controller/registry.hpp"
#include "factory/factory.hpp"

namespace adam
{
    static default_factory<port, port_internal>                 global_port_internal_factory            = default_factory<port, port_internal>();
    static default_factory<processor, filter_frame_aligner>     global_filter_frame_aligner_factory     = default_factory<processor, filter_frame_aligner>();

    module_essential::module_essential() : module("essential"_ct, adam::make_version(1, 0, 0), adam::sdk_version)
    {
        data_format_transparent.set_origin_module(this);

        // Register internal data formats
        m_data_formats.emplace(data_format_transparent.get_name().get_hash(), &data_format_transparent);

        // Register internal port factories
        m_port_factories.emplace    
        (
            port_internal::type_name(),
            registry::factory_data_port(&global_port_internal_factory, nullptr, port_internal::direction)
        );

        // Register internal processors
        m_processor_factories.emplace
        (
            filter_frame_aligner::type_name(),
            registry::factory_data_processor(&global_filter_frame_aligner_factory, &filter_frame_aligner::get_user_parameters())
        );
    }

    module_essential::~module_essential()
    {
    }

    module_essential internal_module_essential = module_essential();
}
