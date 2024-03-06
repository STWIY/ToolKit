#pragma once

#include <Windows.h>
#include <string>

#include "Helpers.hxx"

class FileReader {
public:
    FileReader(std::wstring name) {
        file = CreateFileW(&name[0], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE) {
            return;
        }
        LARGE_INTEGER llsize{ 0 };
        if (!GetFileSizeEx(file, &llsize)) {
            CloseHandle(file); file = nullptr;
            return;
        }
        mapping = CreateFileMappingW(file, NULL, PAGE_READONLY, llsize.HighPart, llsize.LowPart, NULL);
        if (mapping == NULL) {
            CloseHandle(file); file = nullptr;
            return;
        }
        base_address = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
        if (base_address == NULL) {
            CloseHandle(mapping); mapping = nullptr;
            CloseHandle(file); file = nullptr;
            return;
        }
        file_size = llsize.QuadPart;
    }
    ~FileReader() {
        if (base_address) UnmapViewOfFile(base_address);
        if (mapping) CloseHandle(mapping);
        if (file) CloseHandle(file);
    }
    void* data() { return base_address; }
    size_t size() { return file_size; }
    static std::string to_utf8(std::wstring name) {
        auto f = FileReader(name);
        return std::string((char*)f.data(), f.size());
    }
    static std::wstring to_wstr(std::wstring name) {
        return utf8_to_wstr(to_utf8(name));
    }
private:
    HANDLE file = nullptr;
    HANDLE mapping = nullptr;
    PVOID  base_address = nullptr;
    unsigned long long  file_size = 0;
};