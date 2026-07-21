#include "themes.hpp"
#include "../gui-strings.hpp"
#include <imgui.h>
#include <cstring>

using namespace adam::string_hashed_ct_literals;

namespace adam::gui::themes
{
    static void setup_dark_style()
    {
        ImGui::StyleColorsDark();
        
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;
        
        colors[ImGuiCol_Text]                       = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // #f2f2f2ff
        colors[ImGuiCol_TextDisabled]               = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_WindowBg]                   = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1f1f1fff
        colors[ImGuiCol_ChildBg]                    = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // #262626ff
        colors[ImGuiCol_PopupBg]                    = ImVec4(0.12f, 0.12f, 0.12f, 0.96f); // #1f1f1ff5
        colors[ImGuiCol_Border]                     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_BorderShadow]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_FrameBg]                    = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // #2a2a2aff
        colors[ImGuiCol_FrameBgHovered]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_FrameBgActive]              = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // #4c4c4cff
        colors[ImGuiCol_TitleBg]                    = ImVec4(0.08f, 0.08f, 0.08f, 1.00f); // #141414ff
        colors[ImGuiCol_TitleBgActive]              = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // #262626ff
        colors[ImGuiCol_TitleBgCollapsed]           = ImVec4(0.08f, 0.08f, 0.08f, 0.80f); // #141414cc
        colors[ImGuiCol_MenuBarBg]                  = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // #242424ff
        colors[ImGuiCol_ScrollbarBg]                = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1f1f1fff
        colors[ImGuiCol_ScrollbarGrab]              = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // #4c4c4cff
        colors[ImGuiCol_ScrollbarGrabHovered]       = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // #666666ff
        colors[ImGuiCol_ScrollbarGrabActive]        = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_CheckMark]                  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_SliderGrab]                 = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_SliderGrabActive]           = ImVec4(0.46f, 0.79f, 0.98f, 1.00f); // #75c9faff
        colors[ImGuiCol_Button]                     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_ButtonHovered]              = ImVec4(0.35f, 0.35f, 0.35f, 1.00f); // #595959ff
        colors[ImGuiCol_ButtonActive]               = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // #737373ff
        colors[ImGuiCol_Header]                     = ImVec4(0.26f, 0.59f, 0.98f, 0.50f); // #4296fa80
        colors[ImGuiCol_HeaderHovered]              = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); // #4296facc
        colors[ImGuiCol_HeaderActive]               = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_Separator]                  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_SeparatorHovered]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // #666666ff
        colors[ImGuiCol_SeparatorActive]            = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_ResizeGrip]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.25f); // #4296fa40
        colors[ImGuiCol_ResizeGripHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f); // #4296faab
        colors[ImGuiCol_ResizeGripActive]           = ImVec4(0.26f, 0.59f, 0.98f, 0.95f); // #4296faf2
        colors[ImGuiCol_Tab]                        = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // #2e2e2eff
        colors[ImGuiCol_TabHovered]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); // #4296facc
        colors[ImGuiCol_TabActive]                  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_TabUnfocused]               = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // #262626ff
        colors[ImGuiCol_TabUnfocusedActive]         = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // #2a2a2aff
        colors[ImGuiCol_TableHeaderBg]              = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // #2e2e2eff
        colors[ImGuiCol_TableBorderStrong]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_TableBorderLight]           = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // #2a2a2aff
        colors[ImGuiCol_TableRowBg]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_TableRowBgAlt]              = ImVec4(1.00f, 1.00f, 1.00f, 0.03f); // #ffffff08
        colors[ImGuiCol_CheckboxSelectedBg]         = ImVec4(0.23f, 0.23f, 0.23f, 1.00f); // #3b3b3bff
        colors[ImGuiCol_InputTextCursor]            = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // #f2f2f2ff
        colors[ImGuiCol_TabSelected]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_TabSelectedOverline]        = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_TabDimmed]                  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // #262626ff
        colors[ImGuiCol_TabDimmedSelected]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // #2a2a2aff
        colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.50f, 0.50f, 0.50f, 0.00f); // #80808000
        colors[ImGuiCol_PlotLines]                  = ImVec4(0.61f, 0.61f, 0.61f, 1.00f); // #9c9c9cff
        colors[ImGuiCol_PlotLinesHovered]           = ImVec4(1.00f, 0.43f, 0.35f, 1.00f); // #ff6e59ff
        colors[ImGuiCol_PlotHistogram]              = ImVec4(0.90f, 0.70f, 0.00f, 1.00f); // #e6b200ff
        colors[ImGuiCol_PlotHistogramHovered]       = ImVec4(1.00f, 0.60f, 0.00f, 1.00f); // #ff9900ff
        colors[ImGuiCol_TextLink]                   = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_TextSelectedBg]             = ImVec4(0.26f, 0.59f, 0.98f, 0.35f); // #4296fa59
        colors[ImGuiCol_TreeLines]                  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040ff
        colors[ImGuiCol_DragDropTarget]             = ImVec4(1.00f, 1.00f, 0.00f, 0.90f); // #ffff00e6
        colors[ImGuiCol_DragDropTargetBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_UnsavedMarker]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #ffffffff
        colors[ImGuiCol_NavCursor]                  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // #4296faff
        colors[ImGuiCol_NavWindowingHighlight]      = ImVec4(1.00f, 1.00f, 1.00f, 0.70f); // #ffffffb2
        colors[ImGuiCol_NavWindowingDimBg]          = ImVec4(0.80f, 0.80f, 0.80f, 0.20f); // #cccccc33
        colors[ImGuiCol_ModalWindowDimBg]           = ImVec4(0.80f, 0.80f, 0.80f, 0.35f); // #cccccc59
    }

    static void setup_light_style()
    {
        ImGui::StyleColorsLight();
        
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;
        
        colors[ImGuiCol_Text]                       = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1f1f1fff
        colors[ImGuiCol_TextDisabled]               = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_WindowBg]                   = ImVec4(0.96f, 0.96f, 0.96f, 1.00f); // #f5f5f5ff
        colors[ImGuiCol_ChildBg]                    = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_PopupBg]                    = ImVec4(0.96f, 0.96f, 0.96f, 0.98f); // #f5f5f5fa
        colors[ImGuiCol_Border]                     = ImVec4(0.78f, 0.78f, 0.78f, 1.00f); // #c7c7c7ff
        colors[ImGuiCol_BorderShadow]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_FrameBg]                    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #ffffffff
        colors[ImGuiCol_FrameBgHovered]             = ImVec4(0.96f, 0.96f, 0.96f, 1.00f); // #f5f5f5ff
        colors[ImGuiCol_FrameBgActive]              = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // #e6e6e6ff
        colors[ImGuiCol_TitleBg]                    = ImVec4(0.82f, 0.82f, 0.82f, 1.00f); // #d1d1d1ff
        colors[ImGuiCol_TitleBgActive]              = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // #e6e6e6ff
        colors[ImGuiCol_TitleBgCollapsed]           = ImVec4(0.82f, 0.82f, 0.82f, 0.80f); // #d1d1d1cc
        colors[ImGuiCol_MenuBarBg]                  = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_ScrollbarBg]                = ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // #d9d9d9ff
        colors[ImGuiCol_ScrollbarGrab]              = ImVec4(0.70f, 0.70f, 0.70f, 1.00f); // #b2b2b2ff
        colors[ImGuiCol_ScrollbarGrabHovered]       = ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // #999999ff
        colors[ImGuiCol_ScrollbarGrabActive]        = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_CheckMark]                  = ImVec4(0.00f, 0.48f, 1.00f, 1.00f); // #007affff
        colors[ImGuiCol_SliderGrab]                 = ImVec4(0.00f, 0.48f, 1.00f, 1.00f); // #007affff
        colors[ImGuiCol_SliderGrabActive]           = ImVec4(0.00f, 0.35f, 0.85f, 1.00f); // #0059d9ff
        colors[ImGuiCol_Button]                     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #ffffffff
        colors[ImGuiCol_ButtonHovered]              = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // #f2f2f2ff
        colors[ImGuiCol_ButtonActive]               = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // #e6e6e6ff
        colors[ImGuiCol_Header]                     = ImVec4(0.00f, 0.48f, 1.00f, 0.20f); // #007aff33
        colors[ImGuiCol_HeaderHovered]              = ImVec4(0.00f, 0.48f, 1.00f, 0.40f); // #007aff66
        colors[ImGuiCol_HeaderActive]               = ImVec4(0.00f, 0.48f, 1.00f, 0.60f); // #007aff99
        colors[ImGuiCol_Separator]                  = ImVec4(0.78f, 0.78f, 0.78f, 1.00f); // #c7c7c7ff
        colors[ImGuiCol_SeparatorHovered]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // #999999ff
        colors[ImGuiCol_SeparatorActive]            = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080ff
        colors[ImGuiCol_ResizeGrip]                 = ImVec4(0.00f, 0.48f, 1.00f, 0.25f); // #007aff40
        colors[ImGuiCol_ResizeGripHovered]          = ImVec4(0.00f, 0.48f, 1.00f, 0.50f); // #007aff80
        colors[ImGuiCol_ResizeGripActive]           = ImVec4(0.00f, 0.48f, 1.00f, 0.75f); // #007affbf
        colors[ImGuiCol_Tab]                        = ImVec4(0.88f, 0.88f, 0.88f, 1.00f); // #e0e0e0ff
        colors[ImGuiCol_TabHovered]                 = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); // #f0f0f0ff
        colors[ImGuiCol_TabActive]                  = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_TabUnfocused]               = ImVec4(0.88f, 0.88f, 0.88f, 1.00f); // #e0e0e0ff
        colors[ImGuiCol_TabUnfocusedActive]         = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_TableHeaderBg]              = ImVec4(0.88f, 0.88f, 0.88f, 1.00f); // #e0e0e0ff
        colors[ImGuiCol_TableBorderStrong]          = ImVec4(0.75f, 0.75f, 0.75f, 1.00f); // #bfbfbfff
        colors[ImGuiCol_TableBorderLight]           = ImVec4(0.80f, 0.80f, 0.80f, 1.00f); // #ccccccff
        colors[ImGuiCol_TableRowBg]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_TableRowBgAlt]              = ImVec4(0.00f, 0.00f, 0.00f, 0.02f); // #00000005
        colors[ImGuiCol_CheckboxSelectedBg]         = ImVec4(0.93f, 0.93f, 0.93f, 1.00f); // #edededff
        colors[ImGuiCol_InputTextCursor]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1f1f1fff
        colors[ImGuiCol_TabSelected]                = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_TabSelectedOverline]        = ImVec4(0.00f, 0.48f, 1.00f, 0.60f); // #007aff99
        colors[ImGuiCol_TabDimmed]                  = ImVec4(0.88f, 0.88f, 0.88f, 1.00f); // #e0e0e0ff
        colors[ImGuiCol_TabDimmedSelected]          = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #ebebebff
        colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.53f, 0.53f, 0.87f, 0.00f); // #8787de00
        colors[ImGuiCol_PlotLines]                  = ImVec4(0.39f, 0.39f, 0.39f, 1.00f); // #636363ff
        colors[ImGuiCol_PlotLinesHovered]           = ImVec4(1.00f, 0.43f, 0.35f, 1.00f); // #ff6e59ff
        colors[ImGuiCol_PlotHistogram]              = ImVec4(0.90f, 0.70f, 0.00f, 1.00f); // #e6b200ff
        colors[ImGuiCol_PlotHistogramHovered]       = ImVec4(1.00f, 0.45f, 0.00f, 1.00f); // #ff7300ff
        colors[ImGuiCol_TextLink]                   = ImVec4(0.00f, 0.48f, 1.00f, 0.60f); // #007aff99
        colors[ImGuiCol_TextSelectedBg]             = ImVec4(0.26f, 0.59f, 0.98f, 0.35f); // #4296fa59
        colors[ImGuiCol_TreeLines]                  = ImVec4(0.78f, 0.78f, 0.78f, 1.00f); // #c7c7c7ff
        colors[ImGuiCol_DragDropTarget]             = ImVec4(0.26f, 0.59f, 0.98f, 0.95f); // #4296faf2
        colors[ImGuiCol_DragDropTargetBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // #00000000
        colors[ImGuiCol_UnsavedMarker]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f); // #000000ff
        colors[ImGuiCol_NavCursor]                  = ImVec4(0.00f, 0.48f, 1.00f, 0.40f); // #007aff66
        colors[ImGuiCol_NavWindowingHighlight]      = ImVec4(0.70f, 0.70f, 0.70f, 0.70f); // #b2b2b2b2
        colors[ImGuiCol_NavWindowingDimBg]          = ImVec4(0.20f, 0.20f, 0.20f, 0.20f); // #33333333
        colors[ImGuiCol_ModalWindowDimBg]           = ImVec4(0.20f, 0.20f, 0.20f, 0.35f); // #33333359
    }

    gui_theme parse_theme(adam::string_hashed name)
    {
        switch (name.get_hash())
        {
            case "light"_ct: return gui_theme::light;
            case "dark"_ct:  return gui_theme::dark;
            default:         return gui_theme::dark; // Fallback to dark
        }
    }

    adam::string_hashed theme_to_string(gui_theme theme)
    {
        switch (theme)
        {
            case gui_theme::light: return "light"_ct;
            case gui_theme::dark:  return "dark"_ct;
            default:               return "dark"_ct;
        }
    }

    const char* get_theme_name(gui_theme theme, adam::language lang)
    {
        switch (theme)
        {
            case gui_theme::light: return adam::gui::get_gui_string(adam::gui::gui_string_id::theme_light, lang);
            case gui_theme::dark:  return adam::gui::get_gui_string(adam::gui::gui_string_id::theme_dark, lang);
            default:               return adam::gui::get_gui_string(adam::gui::gui_string_id::theme_dark, lang);
        }
    }

    void apply_theme(gui_theme theme)
    {
        switch (theme)
        {
            case gui_theme::light:
            {
                setup_light_style();
                break;
            }
            case gui_theme::dark:
            default:
            {
                setup_dark_style();
                break;
            }
        }
    }
}
