#include "WinMain.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

ID3D11Texture2D* g_BackBufferTexture;

DirectX::SimpleMath::Matrix m_world;
DirectX::SimpleMath::Matrix m_view;
DirectX::SimpleMath::Matrix m_proj;

// Render
namespace Render
{
    void Base()
    {
        ImGui::SetNextWindowPos({ 0.f, 0.f });
        ImGui::SetNextWindowSize(g_ImGuiIO->DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });

        ImGui::Begin("##Base", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

        ImGui::PopStyleVar(3);

        ImGuiID m_DockSpaceID = ImGui::GetID("##BaseDockSpace");
        ImGui::DockSpace(m_DockSpaceID, { 0.f, 0.f }, ImGuiDockNodeFlags_PassthruCentralNode);

        bool m_OpenFile = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(u8"\uF15B File"))
            {
                if (ImGui::MenuItemEx("Open", u8"\uF56F", "CTRL + O"))
                    m_OpenFile = true;

                ImGui::EndMenu();
            }
        }

        // Shortcuts
        if (ImGui::GetCurrentContext()->OpenPopupStack.empty())
        {
            std::pair<std::pair<ImGuiKey, ImGuiKey>, bool*> m_Shortcuts[] =
            {
                { { ImGuiKey_LeftCtrl, ImGuiKey_O }, &m_OpenFile }
            };
            for (auto& m_Pair : m_Shortcuts)
            {
                ImGuiKey m_Key = m_Pair.first.second;
                if (m_Key == ImGuiKey_None)
                    m_Key = m_Pair.first.first;
                else if (!ImGui::IsKeyDown(m_Pair.first.first))
                    continue;

                if (!ImGui::IsKeyPressed(m_Key, false))
                    continue;

                *m_Pair.second = true;
            }
        }

        // Options
        if (m_OpenFile)
        {
            // Example usage
            std::string filePath = g_FileHandler->OpenFileDlg();
            if (!filePath.empty()) {
                // File selected, process it
                g_FileHandler->ProcessFile(std::wstring(filePath.begin(), filePath.end()));
            }
            else {
                // No file selected or dialog canceled
                std::cout << "No file selected or dialog canceled." << std::endl;
            }
        }

        ImGui::End();
    }

    void OnFrame()
    {
        if (g_FileHandler != nullptr)
        {
            // Render different layouts based on file type
            if (g_FileHandler->m_bFileLoaded)
                g_FileHandler->Render();
        }
        else 
        {
            Base();
        }
    }
}

// DirectX
namespace DirectX
{
    static IDXGISwapChain* m_SwapChain = nullptr;
    static ID3D11RenderTargetView* m_RenderTargetView = nullptr;

    void CleanupRenderTarget()
    {
        if (m_RenderTargetView)
        {
            m_RenderTargetView->Release();
            m_RenderTargetView = nullptr;
        }
    }

    void CleanupDevice()
    {
        CleanupRenderTarget();

        if (m_SwapChain)
        {
            m_SwapChain->Release();
            m_SwapChain = nullptr;
        }

        if (g_DeviceCtx)
        {
            g_DeviceCtx->Release();
            g_DeviceCtx = nullptr;
        }

        if (g_DeviceCtx)
        {
            g_DeviceCtx->Release();
            g_DeviceCtx = nullptr;
        }
    }

    void CreateRenderTarget()
    {
        m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&g_BackBufferTexture));

        g_Device->CreateRenderTargetView(g_BackBufferTexture, nullptr, &m_RenderTargetView);
        g_BackBufferTexture->Release();
    }

    bool CreateDevice(HWND p_Window)
    {
        DXGI_SWAP_CHAIN_DESC m_SwapChainDesc = { 0 };
        ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));

        m_SwapChainDesc.BufferCount = 2;
        m_SwapChainDesc.BufferDesc.Width = 0;
        m_SwapChainDesc.BufferDesc.Height = 0;
        m_SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        m_SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_SwapChainDesc.OutputWindow = p_Window;
        m_SwapChainDesc.SampleDesc.Count = 1;
        m_SwapChainDesc.SampleDesc.Quality = 0;
        m_SwapChainDesc.Windowed = TRUE;
        m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        const D3D_FEATURE_LEVEL m_FeatureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL m_FeatureLevelDummy;
        HRESULT m_Result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, m_FeatureLevels, ARRAYSIZE(m_FeatureLevels), D3D11_SDK_VERSION, &m_SwapChainDesc, &m_SwapChain, &g_Device, &m_FeatureLevelDummy, &g_DeviceCtx);
        if (m_Result != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }
}

// ImGui
namespace ImGui
{
    void InitializeFonts()
    {
        g_ImGuiIO->Fonts->AddFontDefault();

        // FontAwesome
        {
            ImFontConfig m_FontConfig;
            m_FontConfig.MergeMode = true;
            m_FontConfig.OversampleH = 1;
            m_FontConfig.GlyphOffset.y += 1.f;
            m_FontConfig.PixelSnapH = true;
            static const ImWchar m_GlyphRange[] = { 0xE000, 0xF8FF, 0 };
            g_ImGuiIO->Fonts->AddFontFromMemoryCompressedTTF(Resource::FontAwesome_compressed_data, Resource::FontAwesome_compressed_size, 12.f, &m_FontConfig, m_GlyphRange);
        }
    }

    void ApplyCustomStyle()
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

// WndProc
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND p_HWND, UINT p_Msg, WPARAM p_WParam, LPARAM p_LParam)
{
    if (ImGui_ImplWin32_WndProcHandler(p_HWND, p_Msg, p_WParam, p_LParam))
        return true;

    switch (p_Msg)
    {
    case WM_SYSCOMMAND:
    {
        if ((p_WParam & 0xFFF0) == SC_KEYMENU)
            return 0;
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    break;
    }

    return DefWindowProcA(p_HWND, p_Msg, p_WParam, p_LParam);
}

std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

DirectX::GeometricPrimitive* g_pSphere = nullptr;
// Main
int WINAPI WinMain(HINSTANCE p_Instance, HINSTANCE p_PrevInstance, char* p_CmdLine, int p_CmdShow)
{
    WNDCLASSEXA m_WndClass = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(0), 0, 0, 0, 0, "PermTool_WndClass", 0 };
    m_WndClass.hIcon = LoadIconA(p_Instance, MAKEINTRESOURCEA(IDI_ICON1));
    if (RegisterClassExA(&m_WndClass) == 0)
    {
        MessageBoxA(0, "Couldn't register window class!", PROJECT_NAME, MB_OK | MB_ICONERROR);
        return 1;
    }

    g_Window = CreateWindowA(m_WndClass.lpszClassName, PROJECT_NAME, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), 100, 100, 1000, 600, nullptr, nullptr, m_WndClass.hInstance, nullptr);
    if (!g_Window)
    {
        MessageBoxA(0, "Couldn't create window!", PROJECT_NAME, MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!DirectX::CreateDevice(g_Window))
    {
        DirectX::CleanupDevice();
        UnregisterClassA(m_WndClass.lpszClassName, m_WndClass.hInstance);

        MessageBoxA(0, "Couldn't create D3D11 Device!", PROJECT_NAME, MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_Window, SW_SHOWDEFAULT);
    UpdateWindow(g_Window);

    ImGui::CreateContext();
    g_ImGuiIO = &ImGui::GetIO();
    {
        g_ImGuiIO->IniFilename = nullptr;
        g_ImGuiIO->LogFilename = nullptr;
        g_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    ImGui::StyleColorsDark();
    ImGui::ApplyCustomStyle();
    ImGui::InitializeFonts();

    ImGui_ImplWin32_Init(g_Window);
    ImGui_ImplDX11_Init(g_Device, g_DeviceCtx);

    // Create sphere
    m_shape = GeometricPrimitive::CreateSphere(g_DeviceCtx);

    m_world = Matrix::Identity;

    bool m_Quit = false;
    while (!m_Quit)
    {
        MSG m_Msg;
        while (PeekMessageA(&m_Msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&m_Msg);
            DispatchMessageA(&m_Msg);
            if (m_Msg.message == WM_QUIT)
                m_Quit = true;
        }
        if (m_Quit)
            break;

        if (GetFocus() != g_Window)
        {
            Sleep(1);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Render::OnFrame();

        ImGui::Render();

        static float m_TargetColorValue = 0.f;
        {
            m_TargetColorValue += g_ImGuiIO->DeltaTime * 0.75f;
            while (m_TargetColorValue > 1.f)
                m_TargetColorValue -= 2.f;
        }
        float m_TargetColorValueAbs = (0.025f + (fabsf(m_TargetColorValue) * 0.1f));
        float m_TargetColor[4] = { m_TargetColorValueAbs, m_TargetColorValueAbs, m_TargetColorValueAbs, 1.f };
        g_DeviceCtx->OMSetRenderTargets(1, &DirectX::m_RenderTargetView, nullptr);
        g_DeviceCtx->ClearRenderTargetView(DirectX::m_RenderTargetView, m_TargetColor);

        // Set viewport
        D3D11_VIEWPORT vp;
        vp.Width = 1000;
        vp.Height = 600;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_DeviceCtx->RSSetViewports(1, &vp);

        m_shape->Draw(m_world, m_view, m_proj);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        DirectX::m_SwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DirectX::CleanupDevice();
    DestroyWindow(g_Window);
    UnregisterClassA(m_WndClass.lpszClassName, m_WndClass.hInstance);
    return 0;
}