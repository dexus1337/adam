/**
 * @file    window-layers.cpp
 * @author  dexus1337
 * @brief   Implementation of the layers, projection and overlays control panel for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-layers.hpp"
#include "../cop-strings.hpp"
#include <imgui.h>
#include <algorithm>
#include <string>

namespace adam::cop
{
    /**
     * @brief Saves the current tile layer order, opacities, and visibilities to configuration parameters.
     * @param ctx Reference to the layers window context.
     */
    static void save_map_layers(layers_window_context& ctx)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            if (ctx.map_layer_params[i].provider && ctx.map_layer_params[i].opacity && ctx.map_layer_params[i].visible)
            {
                ctx.map_layer_params[i].provider->set_value(static_cast<int64_t>(ctx.map_options.tile_layers[i].provider));
                ctx.map_layer_params[i].opacity->set_value(static_cast<double>(ctx.map_options.tile_layers[i].opacity));
                ctx.map_layer_params[i].visible->set_value(ctx.map_options.tile_layers[i].visible);
            }
        }
    }

    void draw_layers_window(layers_window_context& ctx, adam::language lang)
    {
        if (!ctx.p_show_control_panel || !ctx.p_show_control_panel->get_value())
        {
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(340.0f, 250.0f), ImGuiCond_FirstUseEver);
        std::string panel_title = std::string(get_cop_string(lbl_layers_panel, lang)) + "###ControlPanel";
        
        if (!ImGui::Begin(panel_title.c_str(), &ctx.p_show_control_panel->value()))
        {
            ImGui::End();
            return;
        }

        const char* provider_items[] = 
        {
            get_cop_string(provider_cartodb, lang),
            get_cop_string(provider_osm, lang),
            get_cop_string(provider_esri, lang),
            get_cop_string(provider_opentopo, lang)
        };

        ImGui::SeparatorText("Layers");

        bool layers_changed = false;

        // 1. Tile Layers Table with Drag and Drop
        if (ImGui::BeginTable("##LayersTable", 3, ImGuiTableFlags_BordersInnerH))
        {
            ImGui::TableSetupColumn("##drag", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("##layer", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("##opac_col", ImGuiTableColumnFlags_WidthStretch);

            bool drag_drop_finished = false;

            for (int i = 0; i < static_cast<int>(ctx.map_options.tile_layers.size()); ++i)
            {
                auto& layer = ctx.map_options.tile_layers[i];
                ImGui::PushID(i);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImVec2 pos = ImGui::GetCursorPos();
                
                ImGui::Selectable("##row", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, ImGui::GetFrameHeight()));
                
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    ImGui::SetDragDropPayload("DND_MAP_LAYER", &i, sizeof(int));
                    ImGui::Text("Move %s", provider_items[static_cast<int>(layer.provider)]);
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MAP_LAYER", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                    {
                        if (payload->IsPreview())
                        {
                            ImGui::GetWindowDrawList()->AddRectFilled(
                                ImGui::GetItemRectMin(), 
                                ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y + 2.0f), 
                                IM_COL32(255, 255, 0, 255));
                        }
                        if (payload->IsDelivery())
                        {
                            int src = *(const int*)payload->Data;
                            int dst = i;
                            if (src != dst)
                            {
                                if (src < dst)
                                {
                                    std::rotate(ctx.map_options.tile_layers.begin() + src, ctx.map_options.tile_layers.begin() + src + 1, ctx.map_options.tile_layers.begin() + dst + 1);
                                }
                                else
                                {
                                    std::rotate(ctx.map_options.tile_layers.begin() + dst, ctx.map_options.tile_layers.begin() + src, ctx.map_options.tile_layers.begin() + src + 1);
                                }
                                layers_changed = true;
                                drag_drop_finished = true;
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                
                ImGui::SetCursorPos(pos);
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("(=)");
                
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                }

                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##vis", &layer.visible))
                {
                    layers_changed = true;
                }
                ImGui::SameLine();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(provider_items[static_cast<int>(layer.provider)]);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::SliderFloat("##opac", &layer.opacity, 0.0f, 1.0f, "%.2f"))
                {
                    layers_changed = true;
                }

                ImGui::PopID();
                if (drag_drop_finished)
                {
                    break;
                }
            }
            ImGui::EndTable();
        }

        if (layers_changed)
        {
            save_map_layers(ctx);
        }

        // 2. Projection Radio Buttons in 2-Column Grid
        ImGui::SeparatorText("Projection");

        if (ImGui::BeginTable("##ProjectionGrid", 2, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("##ProjCol1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##ProjCol2", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ctx.p_map_projection)
            {
                int current_proj = static_cast<int>(ctx.p_map_projection->get_value());
                if (ImGui::RadioButton(get_cop_string(proj_mercator, lang), current_proj == 1))
                {
                    ctx.p_map_projection->set_value(1);
                }

                ImGui::TableSetColumnIndex(1);
                if (ImGui::RadioButton(get_cop_string(proj_equirectangular, lang), current_proj == 0))
                {
                    ctx.p_map_projection->set_value(0);
                }
            }

            ImGui::EndTable();
        }

        // 3. Overlays Checkboxes in 2-Column Grid
        ImGui::SeparatorText("Overlays");

        if (ImGui::BeginTable("##OverlaysGrid", 2, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Col2", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ctx.p_show_grid)
            {
                bool v = ctx.p_show_grid->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_grid_toggle, lang), &v))
                {
                    ctx.p_show_grid->set_value(v);
                }
            }

            ImGui::TableSetColumnIndex(1);
            if (ctx.p_show_coastlines)
            {
                bool v = ctx.p_show_coastlines->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_coastlines_toggle, lang), &v))
                {
                    ctx.p_show_coastlines->set_value(v);
                }
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ctx.p_show_scale_bar)
            {
                bool v = ctx.p_show_scale_bar->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_scale_bar_toggle, lang), &v))
                {
                    ctx.p_show_scale_bar->set_value(v);
                }
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox(get_cop_string(lbl_compass_toggle, lang), &ctx.map_options.show_compass);

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
