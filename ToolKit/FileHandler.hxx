#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>

#include "FileReader.hxx"
#include "RCF.h"

// 3rdParty (ImGui)
#include "3rdParty/ImGui/imgui.h"
#include "3rdParty/ImGui/imgui_internal.h"
#include "3rdParty/ImGui/imgui_impl_win32.h"
#include "3rdParty/ImGui/imgui_impl_dx11.h"
#include "3rdParty/ImGui/imgui_memory_editor.h"

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
    bool m_bFileLoaded = false;
    bool m_bFileSelected = false;
    struct DirectoryNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<DirectoryNode> Children;
        bool IsDirectory;
    };

    DirectoryNode m_RootNode;
    std::string selectedFilePath;

    virtual ~FileHandler() {}

    virtual void Load(const std::wstring& filePath) = 0;
    virtual void RenderTree() = 0;
    virtual void RenderPropetries() = 0;
    virtual void RenderHex() = 0;
};

std::unique_ptr<FileHandler> g_FileHandler;

class RCFHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading RCF file: " << filePath << std::endl;
        ProcessRCFFile(filePath);
    }
    RCF rcf;
private:
    void PrintRCFData() {
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

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::wcerr << L"Error opening file: " << filePath << std::endl;
            return; // Return empty RCF on error
        }

        // Read header
        file.read(reinterpret_cast<char*>(&rcf.header), sizeof(Header));

        if (strcmp(rcf.header.file_id, "ATG CORE CEMENT LIBRARY") != 0) {
            std::wcerr << L"Error: Not a valid RCF archive." << std::endl;
            return;
        }

        // Read directory entries
        file.seekg(rcf.header.dir_offset);
        rcf.directory.resize(rcf.header.number_files);
        file.read(reinterpret_cast<char*>(rcf.directory.data()), rcf.header.dir_size);


        // Read filename directory entries
        file.seekg(rcf.header.flnames_dir_offset + 8);
        rcf.filename_directory.resize(rcf.header.number_files);
        for (auto& entry : rcf.filename_directory) {
            file.read((char*)&entry.date, sizeof(entry.date));
            file.read((char*)&entry.unk2, sizeof(entry.unk2));
            file.read((char*)&entry.unk3, sizeof(entry.unk3));
            file.read((char*)&entry.path_len, sizeof(entry.path_len));
            char* path_buffer = new char[entry.path_len-1];
            file.read(path_buffer, entry.path_len-1);
            entry.path = std::string(path_buffer, entry.path_len - 1);
            delete[] path_buffer;
            file.read((char*)&entry.padding, sizeof(entry.padding));
        }

        file.close();

        //PrintRCFData();

        m_RootNode.FullPath = std::string(filePath.begin(), filePath.end());
        m_RootNode.FileName = m_RootNode.FullPath.substr(m_RootNode.FullPath.find_last_of('\\') + 1);
        m_RootNode.IsDirectory = true;
        CreateTreeNodesFromPaths(m_RootNode);
        m_bFileLoaded = true;
    }

    bool GetFileInformation(std::string path)
    {
        int index = 0;
        for (const auto& filenameEntry : rcf.filename_directory) {
            if (filenameEntry.path == path)
            {
                printf("File index: %d\n", index);
                printf("File path: %s\n", path.c_str());
                printf("File offset: %d\n", rcf.directory[index].fl_offset);
                printf("File size: %d\n", rcf.directory[index].fl_size);
                printf("File hash: %p\n", rcf.directory[index].hash);
                return true;
            }
            index++;
        }

        return false;
    }

    void CreateTreeNodesFromPaths(DirectoryNode& parentNode) {
        for (const auto& filenameEntry : rcf.filename_directory) {
            std::string directoryPath = filenameEntry.path.substr(0, filenameEntry.path.find_last_of('\\'));

            std::istringstream iss(directoryPath);
            std::string directory;
            DirectoryNode* currentNode = &parentNode;

            while (std::getline(iss, directory, '\\')) {
                bool found = false;
                for (auto& child : currentNode->Children) {
                    if (child.FileName == directory && child.IsDirectory) {
                        currentNode = &child;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    DirectoryNode newNode;
                    newNode.FileName = directory;
                    newNode.FullPath = directory;
                    newNode.IsDirectory = true;
                    currentNode->Children.push_back(newNode);
                    currentNode = &currentNode->Children.back();
                }
            }

            DirectoryNode fileNode;
            fileNode.FileName = filenameEntry.path.substr(filenameEntry.path.find_last_of('\\') + 1);
            fileNode.FullPath = filenameEntry.path;
            fileNode.IsDirectory = false;
            currentNode->Children.push_back(fileNode);
        }
    }


    void DisplayDirectoryNode(const DirectoryNode& parentNode)
    {
        ImGui::PushID(&parentNode);
        if (parentNode.IsDirectory)
        {
            if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
            {
                for (const DirectoryNode& childNode : parentNode.Children)
                    DisplayDirectoryNode(childNode);
                ImGui::TreePop();
            }
        }
        else
        {
            if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
            {
                if (ImGui::IsItemClicked() && selectedFilePath != parentNode.FullPath)
                {
                    m_bFileSelected = false;
                    selectedFilePath = parentNode.FullPath;
                    GetFileInformation(selectedFilePath);
                    printf("Selected file: %s\n", selectedFilePath.c_str());
                    m_bFileSelected = true;
                }
            }
        }
        ImGui::PopID();
    }

    void RenderTree()
    {
        DisplayDirectoryNode(g_FileHandler->m_RootNode);
    }

    void RenderPropetries()
    {
        if (g_FileHandler->m_bFileSelected)
        {

        }
    }

    void RenderHex()
    {
        if (g_FileHandler->m_bFileSelected)
        {

        }
    }
};

class P3DHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading P3D file: " << filePath << std::endl;
        // Add P3D file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

class CSOHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading CSO file: " << filePath << std::endl;
        // Add CSO file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

class BIKHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading BIK file: " << filePath << std::endl;
        // Add BIK file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

class RSDHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath) override {
        std::wcout << "Loading RSD file: " << filePath << std::endl;
        // Add RSD file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
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

        if (extension == L"rcf") {
            g_FileHandler = std::make_unique<RCFHandler>();
        }
        else if (extension == L"p3d") {
            g_FileHandler = std::make_unique<P3DHandler>();
        }
        else if (extension == L"cso") {
            g_FileHandler = std::make_unique<CSOHandler>();
        }
        else if (extension == L"bik") {
            g_FileHandler = std::make_unique<BIKHandler>();
        }
        else if (extension == L"rsd") {
            g_FileHandler = std::make_unique<RSDHandler>();
        }

        if (g_FileHandler) {
            g_FileHandler->Load(filePath);
        }
        else {
            std::wcerr << "Unsupported file type: " << extension << std::endl;
        }
    }
    else {
        std::wcerr << "Invalid file path: " << filePath << std::endl;
    }
}

void RenderTree()
{
    if (g_FileHandler && g_FileHandler->m_bFileLoaded) {
        g_FileHandler->RenderTree();
    }
}

void RenderPropetries()
{
    if (g_FileHandler && g_FileHandler->m_bFileLoaded) {
        g_FileHandler->RenderPropetries();
    }
}

void RenderHex()
{
    if (g_FileHandler && g_FileHandler->m_bFileLoaded) {
        g_FileHandler->RenderHex();
    }
}