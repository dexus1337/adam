#include "cli-settings.hpp"

namespace adam::cli
{
    cli_settings::cli_settings() : adam::configuration_item("cli_settings")
    {
        add_parameter(std::make_unique<adam::configuration_parameter_integer>(
            "terminal_width"_ct, 
            110, 
            60, 
            500
        ));
    }
}
