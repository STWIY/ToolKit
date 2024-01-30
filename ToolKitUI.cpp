
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
        ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]               = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
		colors[ImGuiCol_ChildBg]                = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
		colors[ImGuiCol_PopupBg]                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
		colors[ImGuiCol_Border]                 = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
		colors[ImGuiCol_BorderShadow]           = ImVec4(0.14f, 0.16f, 0.11f, 0.52f);
		colors[ImGuiCol_FrameBg]                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.27f, 0.30f, 0.23f, 1.00f);
		colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.34f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBg]                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
		colors[ImGuiCol_TitleBgActive]          = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg]              = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.28f, 0.32f, 0.24f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.25f, 0.30f, 0.22f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.23f, 0.27f, 0.21f, 1.00f);
		colors[ImGuiCol_CheckMark]              = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_SliderGrab]             = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
		colors[ImGuiCol_Button]                 = ImVec4(0.29f, 0.34f, 0.26f, 0.40f);
		colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		colors[ImGuiCol_ButtonActive]           = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
		colors[ImGuiCol_Header]                 = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		colors[ImGuiCol_HeaderHovered]          = ImVec4(0.35f, 0.42f, 0.31f, 0.6f);
		colors[ImGuiCol_HeaderActive]           = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
		colors[ImGuiCol_Separator]              = ImVec4(0.14f, 0.16f, 0.11f, 1.00f);
		colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
		colors[ImGuiCol_SeparatorActive]        = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_ResizeGrip]             = ImVec4(0.19f, 0.23f, 0.18f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_Tab]                    = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		colors[ImGuiCol_TabHovered]             = ImVec4(0.54f, 0.57f, 0.51f, 0.78f);
		colors[ImGuiCol_TabActive]              = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_TabUnfocused]           = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
		//colors[ImGuiCol_DockingPreview]       = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		//colors[ImGuiCol_DockingEmptyBg]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.78f, 0.28f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_DragDropTarget]         = ImVec4(0.73f, 0.67f, 0.24f, 1.00f);
		colors[ImGuiCol_NavHighlight]           = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1.0f;
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 0.0f;

		ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.f);
	}
}