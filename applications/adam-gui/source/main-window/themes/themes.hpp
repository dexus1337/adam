#pragma once

#include <adam-sdk.hpp>
#include <cstdint>

namespace adam::gui::themes
{
    enum class gui_theme : std::uint8_t
    {
        dark = 0,
        light
    };

    static constexpr std::size_t c_themes_count = 2;

    gui_theme parse_theme(adam::string_hashed name);
    adam::string_hashed theme_to_string(gui_theme theme);
    const char* get_theme_name(gui_theme theme, adam::language lang);
    void apply_theme(gui_theme theme);
}
