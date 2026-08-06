#pragma once
#include "imgui.h"

namespace Theme {

    inline void Apply() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* c = style.Colors;

        style.WindowRounding    = 6.0f;
        style.FrameRounding     = 4.0f;
        style.GrabRounding      = 4.0f;
        style.WindowPadding     = ImVec2(14, 14);
        style.FramePadding      = ImVec2(8, 6);
        style.ItemSpacing       = ImVec2(8, 8);

        c[ImGuiCol_WindowBg]        = ImVec4(0.07f, 0.08f, 0.10f, 1.00f);
        c[ImGuiCol_ChildBg]         = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
        c[ImGuiCol_FrameBg]         = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
        c[ImGuiCol_FrameBgHovered]  = ImVec4(0.20f, 0.19f, 0.14f, 1.00f);
        c[ImGuiCol_FrameBgActive]   = ImVec4(0.25f, 0.21f, 0.10f, 1.00f);
        c[ImGuiCol_TitleBg]         = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
        c[ImGuiCol_TitleBgActive]   = ImVec4(0.10f, 0.08f, 0.03f, 1.00f);
        c[ImGuiCol_Button]          = ImVec4(0.55f, 0.42f, 0.10f, 1.00f);
        c[ImGuiCol_ButtonHovered]   = ImVec4(0.70f, 0.55f, 0.15f, 1.00f);
        c[ImGuiCol_ButtonActive]    = ImVec4(0.80f, 0.63f, 0.18f, 1.00f);
        c[ImGuiCol_Header]          = ImVec4(0.20f, 0.18f, 0.10f, 1.00f);
        c[ImGuiCol_HeaderHovered]   = ImVec4(0.30f, 0.25f, 0.12f, 1.00f);
        c[ImGuiCol_HeaderActive]    = ImVec4(0.35f, 0.28f, 0.13f, 1.00f);
        c[ImGuiCol_CheckMark]       = ImVec4(0.90f, 0.75f, 0.25f, 1.00f);
        c[ImGuiCol_Text]            = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
        c[ImGuiCol_Separator]       = ImVec4(0.30f, 0.28f, 0.20f, 1.00f);
        c[ImGuiCol_Border]          = ImVec4(0.25f, 0.23f, 0.16f, 0.60f);
    }

    inline ImVec4 FactionColor(int factionEnum) {
        switch (factionEnum) {
            case 1: return ImVec4(0.35f, 0.55f, 0.95f, 1.0f);
            case 2: return ImVec4(0.85f, 0.20f, 0.20f, 1.0f);
            default: return ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
        }
    }
}
