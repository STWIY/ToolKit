
#include "ToolKitUI.h"
#include "WinMain.h"
#include <mutex>

namespace ui
{
    const char* m_Explorer = "Explorer";
    const char* m_PropertiesTitle = "Properties";
    const char* m_HexEditorTitle = "Hex Editor";
    static ImGuiID g_DockspaceID = 0;

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
        this->Render();
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
ImGuiID dockId;
    void ToolKitUI::Render()
    {
        bool canNotFocusMain = false;

        ImVec2 wndSize = ImGui::GetIO().DisplaySize;
        ImVec2 wndPadding =  ImGui::GetStyle().WindowPadding;

        ImGui::SetNextWindowSize(wndSize);
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        if(ImGui::Begin("##main_view", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, wndPadding);

            ImGuiID dockspace_id = ImGui::GetID("##main_dockspace");

            static std::once_flag calledBuilderFlag;

            std::call_once(calledBuilderFlag, [&]() {
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id); // Add empty node

                ImGui::DockBuilderDockWindow(m_Explorer, ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.45f, nullptr, nullptr));

                ImGuiID m_PropertiesID = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_None, 0.f, nullptr, nullptr);
                ImGui::DockBuilderDockWindow(m_PropertiesTitle, m_PropertiesID);
                ImGui::DockBuilderDockWindow(m_HexEditorTitle, ImGui::DockBuilderSplitNode(m_PropertiesID, ImGuiDir_Down, 0.45f, nullptr, nullptr));

            });

            ImGui::DockSpace(dockspace_id);

            // Add a menu bar
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    // Add menu items here
                    if (ImGui::MenuItem("Open"))
                    {
                        // Handle "Open" action
                    }
                    if (ImGui::MenuItem("Save"))
                    {
                        // Handle "Save" action
                    }
                    if (ImGui::MenuItem("Exit"))
                    {
                        // Handle "Exit" action
                    }

                    ImGui::EndMenu();
                }

                // Add more menu items here if needed

                ImGui::EndMainMenuBar();
            }

            if(ImGui::Begin(m_Explorer))
            {
                
            }
            ImGui::End();

            if(ImGui::Begin(m_PropertiesTitle))
            {

            }
            ImGui::End();

            if(ImGui::Begin(m_HexEditorTitle))
            {

            }
            ImGui::End();


            ImGui::PopStyleVar();
        
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}