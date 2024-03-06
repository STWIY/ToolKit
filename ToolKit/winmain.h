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
#pragma comment(lib, "d3d11")

// Helpers
#include "Helpers.hxx"

// File handler
#include "FileHandler.hxx"
extern FileHandler* g_FileHandler;

// Defines
#define PROJECT_NAME        "Scarface Tool"

// Resources
#include "resource.h"
#include "FontAwesome.hxx"

// 3rdParty (ImGui)
#include "3rdParty/ImGui/imgui.h"
#include "3rdParty/ImGui/imgui_internal.h"
#include "3rdParty/ImGui/imgui_impl_win32.h"
#include "3rdParty/ImGui/imgui_impl_dx11.h"
#include "3rdParty/ImGui/imgui_memory_editor.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow);