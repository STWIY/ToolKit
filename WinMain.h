#pragma once

#include <Windows.h>
#include <d3d9.h>

// Direct3D structures
extern IDirect3D9 *            g_D3D;
extern IDirect3DDevice9 *      g_D3DDev;
extern D3DPRESENT_PARAMETERS   g_D3Dpp;

void InitD3D();
LRESULT CALLBACK MessageProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow);