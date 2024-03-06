#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "FileReader.hxx"
#include "RCF.h"


class Console {
public:
    Console() {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
    }

    ~Console() {
        FreeConsole();
    }

    void log(const char* format, ...) const {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        std::cout << std::endl;
    }
};

Console console;

// File handler interfaces
class FileHandler {
public:
    virtual ~FileHandler() {}

    virtual void Load(const std::wstring& filePath) = 0;
};

class RCFHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading RCF file: " << filePath << std::endl;
        ProcessRCFFile(filePath);
    }
private:
    void PrintRCFData(const RCF& rcf) {
        // Print header data
        std::wcout << L"File ID: " << rcf.header.file_id << std::endl;
        std::wcout << L"Number of Files: " << rcf.header.number_files << std::endl;

        // Print directory entries
        std::wcout << L"Directory Entries:" << std::endl;
        for (const auto& dirEntry : rcf.directory) {
            std::wcout << L"Hash: " << dirEntry.hash << L", File Offset: " << dirEntry.fl_offset << L", File Size: " << dirEntry.fl_size << std::endl;
        }

        // Print filename directory entries
        std::wcout << L"Filename Directory Entries:" << std::endl;
        for (const auto& filenameEntry : rcf.filename_directory) {
            // Convert timestamp to human-readable format
            std::time_t timestamp = filenameEntry.date;
            std::tm* timeinfo = std::localtime(&timestamp);
            char buffer[80];
            std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

            // Print date and path
            std::wcout << L"Date: " << buffer << std::endl;
            std::cout << "Path: " << filenameEntry.path << std::endl;
        }

    }

    void ProcessRCFFile(const std::wstring& filePath) {
        //RCF rcf;

        //std::ifstream file(filePath, std::ios::binary);
        //if (!file.is_open()) {
        //    std::wcerr << L"Error opening file: " << filePath << std::endl;
        //    return; // Return empty RCF on error
        //}

        //// Read header
        //file.read(reinterpret_cast<char*>(&rcf.header), sizeof(Header));

        //if (strcmp(rcf.header.file_id, "ATG CORE CEMENT LIBRARY") != 0) {
        //    std::wcerr << L"Error: Not a valid RCF archive." << std::endl;
        //    return;
        //}

        //// Read directory entries
        //file.seekg(rcf.header.dir_offset);
        //rcf.directory.resize(rcf.header.number_files);
        //file.read(reinterpret_cast<char*>(rcf.directory.data()), rcf.header.dir_size);


        //// Read filename directory entries
        //file.seekg(rcf.header.flnames_dir_offset + 8);
        //rcf.filename_directory.resize(rcf.header.number_files);
        //for (auto& entry : rcf.filename_directory) {
        //    file.read((char*)&entry.date, sizeof(entry.date));
        //    file.read((char*)&entry.unk2, sizeof(entry.unk2));
        //    file.read((char*)&entry.unk3, sizeof(entry.unk3));
        //    file.read((char*)&entry.path_len, sizeof(entry.path_len));
        //    char* path_buffer = new char[entry.path_len-1];
        //    file.read(path_buffer, entry.path_len-1);
        //    entry.path = std::string(path_buffer, entry.path_len - 1);
        //    delete[] path_buffer;
        //    file.read((char*)&entry.padding, sizeof(entry.padding));
        //}

        //file.close();

        RCF rcf;

        FileReader fileReader(filePath);
        if (fileReader.data() == nullptr) {
            std::wcerr << L"Error opening file: " << filePath << std::endl;
            return; // Return empty RCF on error
        }

        // Read header
        if (fileReader.size() < sizeof(Header)) {
            std::wcerr << L"Error: File is too small to contain header." << std::endl;
            return;
        }
        memcpy(&rcf.header, fileReader.data(), sizeof(Header));

        if (strcmp(rcf.header.file_id, "ATG CORE CEMENT LIBRARY") != 0) {
            std::wcerr << L"Error: Not a valid RCF archive." << std::endl;
            return;
        }

        const char* dirDataPtr = static_cast<const char*>(fileReader.data()) + rcf.header.dir_offset;
        rcf.directory.resize(rcf.header.number_files);
        memcpy(rcf.directory.data(), dirDataPtr, rcf.header.dir_size);

        const char* filenameDataPtr = static_cast<const char*>(fileReader.data()) + rcf.header.flnames_dir_offset + 8;
        rcf.filename_directory.resize(rcf.header.number_files);
        for (auto& entry : rcf.filename_directory) {
            memcpy(&entry.date, filenameDataPtr, sizeof(entry.date));
            filenameDataPtr += sizeof(entry.date);
            memcpy(&entry.unk2, filenameDataPtr, sizeof(entry.unk2));
            filenameDataPtr += sizeof(entry.unk2);
            memcpy(&entry.unk3, filenameDataPtr, sizeof(entry.unk3));
            filenameDataPtr += sizeof(entry.unk3);
            memcpy(&entry.path_len, filenameDataPtr, sizeof(entry.path_len));
            filenameDataPtr += sizeof(entry.path_len);
            entry.path = std::string(filenameDataPtr, entry.path_len - 1);
            filenameDataPtr += entry.path_len - 1;
            memcpy(&entry.padding, filenameDataPtr, sizeof(entry.padding));
            filenameDataPtr += sizeof(entry.padding);
        }

        PrintRCFData(rcf);
    }
};

class P3DHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading P3D file: " << filePath << std::endl;
        // Add P3D file loading logic here
    }
};

class CSOHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading CSO file: " << filePath << std::endl;
        // Add CSO file loading logic here
    }
};

class BIKHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading BIK file: " << filePath << std::endl;
        // Add BIK file loading logic here
    }
};

class RSDHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading RSD file: " << filePath << std::endl;
        // Add RSD file loading logic here
    }
};

// Function to open a file dialog and return the selected file path
std::string OpenFileDlg() {
    // Initialize the OPENFILENAMEA structure
    static OPENFILENAMEA m_OpenFileName = { 0 };
    char szFileName[MAX_PATH] = { 0 };

    m_OpenFileName.lStructSize = sizeof(OPENFILENAMEA);
    m_OpenFileName.lpstrFilter = "All Files\0*.*\0";
    m_OpenFileName.lpstrFile = szFileName;
    m_OpenFileName.nMaxFile = MAX_PATH;
    m_OpenFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    // Display the file dialog
    if (GetOpenFileNameA(&m_OpenFileName)) {
        return std::string(szFileName);
    }
    else {
        return "";
    }
}

// Function to determine the file type and choose the appropriate handler
void ProcessFile(const std::wstring& filePath) {
    size_t pos = filePath.find_last_of(L".");
    if (pos != std::wstring::npos) {
        std::wstring extension = filePath.substr(pos + 1);
        std::unique_ptr<FileHandler> handler;

        if (extension == L"rcf") {
            handler = std::make_unique<RCFHandler>();
        }
        else if (extension == L"p3d") {
            handler = std::make_unique<P3DHandler>();
        }
        else if (extension == L"cso") {
            handler = std::make_unique<CSOHandler>();
        }
        else if (extension == L"bik") {
            handler = std::make_unique<BIKHandler>();
        }
        else if (extension == L"rsd") {
            handler = std::make_unique<RSDHandler>();
        }

        if (handler) {
            handler->Load(filePath);
        }
        else {
            std::wcerr << "Unsupported file type: " << extension << std::endl;
        }
    }
    else {
        std::wcerr << "Invalid file path: " << filePath << std::endl;
    }
}