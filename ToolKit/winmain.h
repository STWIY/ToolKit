#pragma once

#define _CRT_SECURE_NO_WARNINGS

// 3rdParty (Half-Float)
#include "3rdParty/umHalf.h"

#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <shlobj_core.h>
#include <Windows.h>
#include <d3d11.h>
#include <sstream>
#pragma comment(lib, "d3d11")

// Helpers
#include "Helpers.hxx"

const char* g_TreeTitle = u8"\uE1D2 Tree";
const char* g_PropertiesTitle = u8"\uF1DE Properties";
const char* g_HexEditorTitle = u8"\uE33B Hex Editor";

// 3rdParty (ImGui)
#include "3rdParty/ImGui/imgui.h"
#include "3rdParty/ImGui/imgui_internal.h"
#include "3rdParty/ImGui/imgui_impl_win32.h"
#include "3rdParty/ImGui/imgui_impl_dx11.h"
#include "3rdParty/ImGui/imgui_memory_editor.h"

// Globals
HWND g_Window = nullptr;
ImGuiIO* g_ImGuiIO = nullptr;

// File handler
#include "filehandler.hxx"

// Defines
#define PROJECT_NAME        "Scarface Tool"

// Resources
#include "resource.h"
#include "FontAwesome.hxx"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow);