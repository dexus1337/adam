#pragma once

#include <adam-sdk.hpp>

namespace adam::cli
{
    class cli_settings : public adam::configuration_item
    {
    public:
        cli_settings();
        ~cli_settings() override = default;
    };
}
