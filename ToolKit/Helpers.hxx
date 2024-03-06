#pragma once

#include <Windows.h>
#include <string>

std::wstring utf8_to_wstr(std::string str)
{
    if (str.empty()) return L"";
    size_t size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}