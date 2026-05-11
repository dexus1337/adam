#include "main-window.hpp"

#include <imgui.h>

namespace adam::gui 
{
    main_window::main_window(gui_controller& ctrl) : m_ctrl(ctrl)
    {
    }

    main_window::~main_window()
    {
    }

    void main_window::render()
    {
        ImGuiIO& io = ImGui::GetIO();
        
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        
        ImGui::Begin("Main UI", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::Text("ADAM Control Panel");
        ImGui::Separator();
        
        if (m_ctrl.is_commander_active())
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Commander connected.");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Commander disconnected.");
            
        float log_height = 250.0f;
        ImGui::SetCursorPosY(io.DisplaySize.y - log_height);
        ImGui::Separator();
        ImGui::Text("Console Output");
        
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        auto log_history = m_ctrl.get_log_history();
        for (const auto& log_line : log_history)
        {
            ImGui::Text("[%llu]", static_cast<unsigned long long>(log_line.timestamp));
            ImGui::SameLine();

            ImVec4 level_color;
            const char* level_text = "";
            switch (log_line.level)
            {
                case adam::log::trace:   level_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); level_text = "[TRACE]"; break;
                case adam::log::info:    level_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); level_text = "[INFO ]"; break;
                case adam::log::warning: level_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); level_text = "[WARN ]"; break;
                case adam::log::error:   level_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); level_text = "[ERROR]"; break;
                case adam::log::fatal:   level_color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f); level_text = "[FATAL]"; break;
                default:                 level_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); level_text = "[UNKWN]"; break;
            }
            
            ImGui::TextColored(level_color, "%s", level_text);
            ImGui::SameLine();
            ImGui::TextUnformatted(log_line.text.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
            
        ImGui::EndChild();
        ImGui::End();
    }
}