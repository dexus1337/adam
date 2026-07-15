#include "module/internals/essential/data/formats/data-format-transparent.hpp"

#include "module/internals/essential/module-essential.hpp"

namespace adam
{
    data_format data_format_transparent = data_format( "transparent", nullptr, nullptr, nullptr, &internal_module_essential );
}