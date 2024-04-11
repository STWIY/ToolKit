#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cerrno>

class FileHandler {
public:

    // File enumeration for each different supported file type
    enum eFileType
    {
        RCF_FILE,
        P3D_FILE,
        RSD_FILE,
        CSO_FILE,
        BIK_FILE,
        FSC_FILE,
        DIR_FILE,
        UNK_FILE
    };

    // Loaded file from disc
    eFileType m_LoadedFileType = eFileType::UNK_FILE;

    // Loaded file path from disc
    std::string m_LoadedFilePath;

    // Loaded file name
    std::string m_LoadedFileName;

    // Identifier if file has been loaded
    bool m_bFileLoaded = false;

    // Selected file from the Tree nodes
    std::string m_selectedFilePath;

    // Binary content of the selected file
    std::vector<uint8_t> m_selectedfileContent;
    int m_selectedFileSize;

    std::string m_savedFilePath;

    virtual void LoadFile(std::string& filePath, int offset = -1) = 0;

    virtual void Render() = 0;

    // Gets the file type by its string extension
    eFileType GetFileTypeFromExtension(std::string extension) {
        static std::unordered_map<std::string, eFileType> extensionMap = {
            {"rcf", RCF_FILE},
            {"p3d", P3D_FILE},
            {"rsd", RSD_FILE},
            {"cso", CSO_FILE},
            {"bik", BIK_FILE},
            {"fsc", FSC_FILE},
            {"dir", DIR_FILE}
        };

        auto it = extensionMap.find(extension);
        if (it != extensionMap.end()) {
            return it->second;
        }
        else {
            return UNK_FILE;
        }
    }

    // Gets the type string extension by its type
    std::string GetFileExtensionFromType(eFileType type) {
        static std::unordered_map<eFileType, std::string> extensionMap = {
            {RCF_FILE, "rcf"},
            {P3D_FILE, "p3d"},
            {RSD_FILE, "rsd"},
            {CSO_FILE, "cso"},
            {BIK_FILE, "bik"},
            {FSC_FILE, "fsc"},
            {DIR_FILE, "dir"}
        };

        auto it = extensionMap.find(type);
        if (it != extensionMap.end()) {
            return it->second;
        }
        else {
            return "unkown";
        }
    }

    // Extract file name from string path
    std::string ExtractFileName(std::string& filePath)
    {
        size_t found = filePath.find_last_of("/\\");
        if (found != std::string::npos) {
            return filePath.substr(found + 1);
        }
        return filePath;
    }

    // Extrad file name from string path with out file extension
    std::string ExtractFileNameWithoutExtension(std::string& filePath)
    {
        size_t foundSlash = filePath.find_last_of("/\\");
        size_t foundDot = filePath.find_last_of(".");

        if (foundSlash != std::string::npos) {
            if (foundDot != std::string::npos && foundDot > foundSlash) {
                return filePath.substr(foundSlash + 1, foundDot - foundSlash - 1);
            }
            return filePath.substr(foundSlash + 1);
        }

        if (foundDot != std::string::npos) {
            return filePath.substr(0, foundDot);
        }

        return filePath;
    }

    // Function to open a file dialog and return the selected file path
    std::string OpenFileDlg()
    {
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

    // Extract file extension from string path
    std::string GetFileExtension(std::string& filePath) {
        size_t dotPos = filePath.find_last_of(L'.');
        if (dotPos != std::string::npos) {
            return filePath.substr(dotPos + 1);
        }
        return "";
    }

    // Save binary data file into a temp file
    void SaveToTempFile(std::string fileName, int size)
    {
        if (m_selectedfileContent.empty()) return;

        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        std::filesystem::path tempFilePath = tempDir / fileName;

        FILE* tempFile = nullptr;
        errno_t err = fopen_s(&tempFile, tempFilePath.string().c_str(), "wb");
        if (err != 0 || tempFile == nullptr) {
            // Print detailed error message using errno and strerror
            std::cerr << "Failed to create temporary file: " << strerror(errno) << std::endl;
            return;
        }

        // Write the content to the temporary file
        fwrite(m_selectedfileContent.data(), size, 1, tempFile);

        // Close the temporary file
        fclose(tempFile);

        m_savedFilePath = tempFilePath.string().c_str(); // Assign the correct path
    }

    // Gets from disc file using its path, offset and size the binary content of the file
    void GetFileContent(std::string filePath, int offset, int size)
    {
        // Read the content from the specified offset and size
        FILE* m_File = nullptr;
        errno_t err = fopen_s(&m_File, filePath.c_str(), "rb");
        if (err != 0 || m_File == nullptr) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        // Seek to the provided offset if provided
        if (fseek(m_File, offset, SEEK_SET) != 0) {
            std::wcerr << L"Error seeking to offset: " << offset << std::endl;
            fclose(m_File);
            return;
        }

        // Read
        m_selectedfileContent.resize(size);
        fread(m_selectedfileContent.data(), size, 1, m_File);
        m_selectedFileSize = size;

        // Close
        fclose(m_File);
    }

    // Default function used for each different handler
    void ProcessFile(std::string filePath, int offset = -1);

    bool ImGui_ToolTipHover()
    {
        return (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip) && ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None));
    }
};

std::unique_ptr<FileHandler> g_FileHandler;

#include "rcf/RCFHandler.hxx"
#include "p3d/P3DHandler.hxx"

void FileHandler::ProcessFile(std::string filePath, int offset)
{
    std::string extension = "";
    std::string m_selectedFilePath = (offset != -1) ? m_selectedFilePath : "";
    extension = GetFileExtension(filePath);
    eFileType type = GetFileTypeFromExtension(extension);

    switch (type)
    {
        case eFileType::RCF_FILE: 
            g_FileHandler = std::make_unique<RCFHandler>(); 
            break;
        case eFileType::P3D_FILE: 
            g_FileHandler = std::make_unique<P3DHandler>(); 
            break;
        default: break;
    }

    if (g_FileHandler) {
        g_FileHandler->m_LoadedFileType = type;
        g_FileHandler->m_selectedFilePath = m_selectedFilePath;
        g_FileHandler->LoadFile(filePath, offset);
    }
}

#endif // FILE_HANDLER_H