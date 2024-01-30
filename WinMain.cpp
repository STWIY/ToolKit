
#include "WinMain.h"
#include "ToolKitUI.h"

// Define the window class name globally
#define WINDOW_CLASS_NAME "ImGuiDX9"

// Direct3D structures
IDirect3D9 *            g_D3D = NULL;
IDirect3DDevice9 *      g_D3DDev = NULL;
D3DPRESENT_PARAMETERS   g_D3Dpp;

 // Global instance of ToolKitUI
ui::ToolKitUI g_ToolKitUI;

// D3D states initialization function
void InitD3D()
{
    // Set viewport
    D3DVIEWPORT9 vp = {0,0, g_D3Dpp.BackBufferWidth,g_D3Dpp.BackBufferHeight, 0,1};
    g_D3DDev->SetViewport(&vp);

    // Set D3D matrices
    D3DMATRIX matId = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    g_D3DDev->SetTransform(D3DTS_WORLD, &matId);
    g_D3DDev->SetTransform(D3DTS_VIEW, &matId);
    D3DMATRIX matProj = { (float)g_D3Dpp.BackBufferHeight/g_D3Dpp.BackBufferWidth,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    g_D3DDev->SetTransform(D3DTS_PROJECTION, &matProj);

    // Disable lighting and culling
    g_D3DDev->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_D3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    g_D3DDev->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 MessageProc callback
LRESULT CALLBACK MessageProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Send event message to ImGui
    if (ImGui_ImplWin32_WndProcHandler(wnd, msg, wParam, lParam))
        return true;

    switch( msg )
    {
    case WM_CHAR:       
        if( wParam==VK_ESCAPE )
            PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:   // Window size has been changed
        // Reset D3D device
        if( g_D3DDev )
        {
            g_D3Dpp.BackBufferWidth  = LOWORD(lParam);
            g_D3Dpp.BackBufferHeight = HIWORD(lParam);
            if( g_D3Dpp.BackBufferWidth>0 && g_D3Dpp.BackBufferHeight>0 )
            {
                // Release ImGui resources and reset D3D device
                ImGui_ImplDX9_InvalidateDeviceObjects();
                g_D3DDev->Reset(&g_D3Dpp);
                InitD3D();  // re-initialize D3D states
                ImGui_ImplDX9_CreateDeviceObjects();
            }
            // TwWindowSize has been called by TwEventWin32, 
            // so it is not necessary to call it again here.
        }
        return 0;
    default:
        return DefWindowProc(wnd, msg, wParam, lParam);
    }
}


// Main
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow)
{
    // Register our window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_CLASSDC|CS_DBLCLKS, MessageProc, 0L, 0L, 
                        instance, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME, NULL };
    RegisterClassEx(&wcex);

    // Create a window
    const int W = 640;
    const int H = 480;
    BOOL fullscreen = FALSE;
    RECT rect = { 0, 0, W, H };
    DWORD style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rect, style, FALSE);
    HWND wnd = CreateWindow(WINDOW_CLASS_NAME, "Scarface: The World Is Yours - DirectX9 ToolKit", 
                        style, CW_USEDEFAULT, CW_USEDEFAULT, 
                        rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, instance, NULL);
    if( !wnd )
    {
        DWORD errorCode = GetLastError();

        LPVOID errorMsgBuffer;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errorCode,
            0, // Default language
            (LPSTR)&errorMsgBuffer,
            0,
            NULL
        );

        // Display the error message with the error code
        MessageBox(NULL, (LPCTSTR)errorMsgBuffer, "Error", MB_OK | MB_ICONERROR);

        // Free the buffer allocated by FormatMessage
        LocalFree(errorMsgBuffer);
    }
    ShowWindow(wnd, cmdShow);
    UpdateWindow(wnd);

    // Initialize Direct3D
    g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
    if( !g_D3D )
    {
        MessageBox(wnd, "Cannot initialize DirectX", "Error", MB_OK|MB_ICONERROR);
        return FALSE;
    }

    // Create a Direct3D device
    ZeroMemory( &g_D3Dpp, sizeof(D3DPRESENT_PARAMETERS) );
    g_D3Dpp.Windowed = !fullscreen;
    if( fullscreen )
    {
        g_D3Dpp.BackBufferWidth = W;
        g_D3Dpp.BackBufferHeight = H;
    }
    g_D3Dpp.BackBufferCount = 1;
    g_D3Dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    if( fullscreen )
        g_D3Dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    else
        g_D3Dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_D3Dpp.hDeviceWindow = wnd;

    g_D3Dpp.EnableAutoDepthStencil = TRUE;
    g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_D3Dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    HRESULT hr = g_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, 
                                     D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_D3Dpp, &g_D3DDev);
    if( FAILED(hr) )
    {
        //DXTRACE_ERR_MSGBOX("Cannot create DirectX device", hr);
        MessageBox(wnd, "Cannot create DirectX device", "Error", MB_OK|MB_ICONERROR);
        g_D3D->Release();
        g_D3D = NULL;
        return FALSE;
    }

    // This example draws a moving strip;
    // create a buffer of vertices for the strip
    struct Vertex
    {
        float x, y, z;
        DWORD color;
    };
    Vertex vertices[2002];
    int numSec = 100;            // number of strip sections
    float color[] = { 1, 0, 0 }; // strip color
    unsigned int bgColor = D3DCOLOR_ARGB(255, 128, 196, 196); // background color

    // Init some D3D states
    InitD3D();

    // Initialize ToolKitUI
    g_ToolKitUI.Initialize(wnd);

    g_ToolKitUI.ApplyTheme();

    bool show_demo_window = true;

    // Main loop
    bool quit = false;
    DWORD t0 = GetTickCount();
    while( !quit )
    {
        // Clear screen and begin draw
        g_D3DDev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, bgColor, 1.0f, 0);
        g_D3DDev->BeginScene();

        // Draw UI
        g_ToolKitUI.EndRender();

        // End draw
        g_D3DDev->EndScene();

        // Present frame buffer
        g_D3DDev->Present(NULL, NULL, NULL, NULL);

        // Process windows messages
        MSG msg;
        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            if( msg.message==WM_QUIT )
                quit = true;
            else if( !TranslateAccelerator(msg.hwnd, NULL, &msg) ) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    // Shutdown ToolKitUI
    g_ToolKitUI.Shutdown();

    // Release Direct3D
    g_D3DDev->Release();
    g_D3DDev = NULL;
    g_D3D->Release();
    g_D3D = NULL;

    return 0;
}