
#include "ToolKitUI.h"
#include "WinMain.h"

namespace ui
{
    void ToolKitUI::Initialize(HWND wnd)
	{
        this->wnd = wnd;
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Dear ImGui style
        ApplyTheme();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(this->wnd);
        ImGui_ImplDX9_Init(g_D3DDev);
    }

	void ToolKitUI::EndRender()
	{
        // Render ImGui
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show simple menu.
        {
            ImGui::Begin("The Hello, world is yours!");

            ImGui::Text("Scarface: The World Is Yours - ToolKit");
 
            ImGui::End();
        }
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	void ToolKitUI::Shutdown()
	{
        // Shutdown ImGui
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
	}

	void ToolKitUI::ApplyTheme()
	{
        ImGuiStyle& m_Style = ImGui::GetStyle();
        {
            m_Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.125f, 0.125f, 0.125f, 1.f);
            m_Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
            m_Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.40f);
            m_Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.67f);
            m_Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            m_Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
            m_Style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
            m_Style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
            m_Style.Colors[ImGuiCol_Button] = ImVec4(0.39f, 0.39f, 0.39f, 0.40f);
            m_Style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
            m_Style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
            m_Style.Colors[ImGuiCol_Header] = ImVec4(0.98f, 0.98f, 0.98f, 0.31f);
            m_Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.59f, 0.59f, 0.59f, 0.80f);
            m_Style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
            m_Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.78f);
            m_Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
            m_Style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.f, 0.f, 0.f, 0.f);
            m_Style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.67f);
            m_Style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.59f, 0.59f, 0.95f);
            m_Style.Colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 0.86f);
            m_Style.Colors[ImGuiCol_TabHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.80f);
            m_Style.Colors[ImGuiCol_TabActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
            m_Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.20f, 0.20f, 0.97f);
            m_Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
            m_Style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.39f, 0.39f, 0.39f, 0.85f);
            m_Style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.f, 1.f, 1.f, 0.85f);
            m_Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.5f, 0.5f, 0.5f, 0.85f);

            m_Style.WindowMenuButtonPosition = ImGuiDir_None;

            m_Style.ScrollbarRounding = 0.f;
        }
	}
}