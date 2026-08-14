#pragma once

#include <adam-sdk.hpp>
#include <cstdint>

namespace adam::lib::imgui
{
    enum class gui_theme : std::uint8_t
    {
        dark = 0,
        light,
        dark_navy
    };

    static constexpr std::size_t c_themes_count = 3;

    gui_theme parse_theme(adam::string_hash name);
    adam::string_hashed_ct theme_to_string(gui_theme theme);
    void apply_theme(gui_theme theme);
}
