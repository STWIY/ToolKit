#pragma once

#include <Windows.h>
#include <iostream>
#include <sstream>

#include "RCF.h"
#include "P3D.h"

// 3rdParty (ImGui)
#include "3rdParty/ImGui/imgui.h"
#include "3rdParty/ImGui/imgui_internal.h"
#include "3rdParty/ImGui/imgui_impl_win32.h"
#include "3rdParty/ImGui/imgui_impl_dx11.h"
#include "3rdParty/ImGui/imgui_memory_editor.h"

#include "texture.hxx"
#include "shader.hxx"
#include "geometry.hxx"
#include "composeitedrawable.hxx"
#include "skeleton.hxx"

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
    enum eFileType
    {
        RCF_FILE,
        P3D_FILE,
        RSD_FILE,
        CSO_FILE,
        BIK_FILE,
        FSC_FILE,
        UNK_FILE
    };

    struct DirectoryNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<DirectoryNode> Children;
        bool IsDirectory; 
        std::streampos file_offset;
        bool IsSelected;
    };

    bool m_bFileLoaded = false;
    bool m_bIsSubFile = false;
    bool m_bFileSelected = false;

    std::wstring m_workingFilePath;
    eFileType m_eWorkingFileType;

    std::string m_selectedFilePath;
    eFileType m_eSelectedFileType;
    std::vector<char> m_selectedfileContent;
    int m_selectedFileSize;

    DirectoryNode* m_RootNode;

    virtual ~FileHandler() {}

    virtual void Load(const std::wstring& filePath, int offset = -1) = 0;
    virtual void RenderTree() = 0;
    virtual void RenderPropetries() = 0;
    virtual void RenderHex() = 0;

    static eFileType GetFileType(const std::wstring& ext) {
        std::wstring lowerExt = ext;
        // Convert the extension to lowercase for case-insensitive comparison
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), towlower);

        if (lowerExt == L"rcf") {
            return eFileType::RCF_FILE;
        }
        else if (lowerExt == L"p3d") {
            return eFileType::P3D_FILE;
        }
        else if (lowerExt == L"rsd") {
            return eFileType::RSD_FILE;
        }
        else if (lowerExt == L"cso") {
            return eFileType::CSO_FILE;
        }
        else if (lowerExt == L"bik") {
            return eFileType::BIK_FILE;
        }
        else if (lowerExt == L"fsc") {
            return eFileType::FSC_FILE;
        }
        else {
            return eFileType::UNK_FILE;
        }
    }

    void GetFileContent(std::wstring filePath, int offset, int size)
    {
        // Read the content from the specified offset and size
        FILE* m_File = nullptr;
        errno_t err = _wfopen_s(&m_File, filePath.c_str(), L"rb");
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

    std::wstring StringToWideString(const std::string& s) {
        std::string curLocale = setlocale(LC_ALL, "");
        const char* _Source = s.c_str();
        size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
        wchar_t* _Dest = new wchar_t[_Dsize];
        wmemset(_Dest, 0, _Dsize);
        mbstowcs(_Dest, _Source, _Dsize);
        std::wstring result = _Dest;
        delete[]_Dest;
        setlocale(LC_ALL, curLocale.c_str());
        return result;
    }

    std::string ExtractFileName(const std::string& filePath) 
    {
        size_t found = filePath.find_last_of("/\\");
        if (found != std::string::npos) {
            return filePath.substr(found + 1);
        }
        return filePath;
    }
};

std::unique_ptr<FileHandler> g_FileHandler;
void ProcessFile(const std::wstring filePath, int offset);

class RCFHandler : public FileHandler {
public:
    RCF rcf;

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

    void Load(const std::wstring& filePath, int offset = -1) override {
        std::wcout << L"Loading RCF file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_workingFilePath = filePath;

        FILE* m_File = nullptr;
        errno_t err = _wfopen_s(&m_File, filePath.c_str(), L"rb");
        if (err != 0 || m_File == nullptr) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        // Seek to the provided offset if provided
        if (offset != -1) {
            std::wcout << L"Seek to offset: " << offset << std::endl;
            if (fseek(m_File, offset, SEEK_SET) != 0) {
                std::wcerr << L"Error seeking to offset: " << offset << std::endl;
                fclose(m_File);
                return;
            }
        }

        // Read header
        fread(&rcf.header, sizeof(RCFHeader), 1, m_File);

        if (strcmp(rcf.header.file_id, "ATG CORE CEMENT LIBRARY") != 0) {
            std::wcerr << L"Error: Not a valid RCF archive." << std::endl;
            fclose(m_File);
            return;
        }

        // Read directory entries
        fseek(m_File, rcf.header.dir_offset, SEEK_SET);
        rcf.directory.resize(rcf.header.number_files);
        fread(rcf.directory.data(), sizeof(RCFDirectoryEntry), rcf.header.number_files, m_File);

        // Sort directory entries by file offset
        std::sort(rcf.directory.begin(), rcf.directory.end(), [](const RCFDirectoryEntry& a, const RCFDirectoryEntry& b) {
            return a.fl_offset < b.fl_offset;
            });

        // Read filename directory entries
        fseek(m_File, (offset == -1) ? rcf.header.flnames_dir_offset + 8 : rcf.header.flnames_dir_offset + 8 + offset, SEEK_SET);
        rcf.filename_directory.resize(rcf.header.number_files);
        for (auto& entry : rcf.filename_directory) {
            fread(&entry.date, sizeof(entry.date), 1, m_File);
            fread(&entry.unk2, sizeof(entry.unk2), 1, m_File);
            fread(&entry.unk3, sizeof(entry.unk3), 1, m_File);
            fread(&entry.path_len, sizeof(entry.path_len), 1, m_File);
            char* path_buffer = new char[entry.path_len - 1];
            fread(path_buffer, entry.path_len - 1, 1, m_File);
            entry.path = std::string(path_buffer, entry.path_len - 1);
            delete[] path_buffer;
            fread(&entry.padding, sizeof(entry.padding), 1, m_File);
        }

        // Close the file
        fclose(m_File);

        m_RootNode = new DirectoryNode();

        m_RootNode->FullPath = (offset == -1) ? std::string(filePath.begin(), filePath.end()) : std::string(m_selectedFilePath.begin(), m_selectedFilePath.end());
        m_RootNode->FileName = m_RootNode->FullPath.substr(m_RootNode->FullPath.find_last_of('\\') + 1);
        m_RootNode->IsDirectory = true;
        CreateTreeNodesFromPaths(m_RootNode);
        m_bFileLoaded = true;
    }


    void CreateTreeNodesFromPaths(DirectoryNode* parentNode) {
        for (const auto& filenameEntry : rcf.filename_directory) {
            std::string directoryPath = filenameEntry.path.substr(0, filenameEntry.path.find_last_of('\\'));

            std::string fileName = g_FileHandler->ExtractFileName(filenameEntry.path);

            std::istringstream iss(directoryPath);
            std::string directory;
            DirectoryNode* currentNode = parentNode;
            if (directoryPath != fileName)
            {
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
            }

            DirectoryNode fileNode;
            fileNode.FileName = filenameEntry.path.substr(filenameEntry.path.find_last_of('\\') + 1);
            fileNode.FullPath = filenameEntry.path;
            fileNode.IsDirectory = false;
            currentNode->Children.push_back(fileNode);
        }
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

                std::wstring wPath(path.begin(), path.end());

                GetFileContent(g_FileHandler->m_workingFilePath, rcf.directory[index].fl_offset, rcf.directory[index].fl_size);

                ProcessFile(g_FileHandler->m_workingFilePath, rcf.directory[index].fl_offset);
                return true;
            }
            index++;
        }

        return false;
    }

    void DisplayDirectoryNode(const DirectoryNode* parentNode)
    {
        if (parentNode != nullptr)
        {
            ImGui::PushID(parentNode);
            if (parentNode->IsDirectory)
            {
                if (ImGui::TreeNodeEx(parentNode->FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
                {
                    for (const DirectoryNode& childNode : parentNode->Children)
                        DisplayDirectoryNode(&childNode);
                    ImGui::TreePop();
                }
            }
            else
            {
                if (ImGui::TreeNodeEx(parentNode->FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
                {
                    if (ImGui::IsItemClicked(0) && g_FileHandler->m_selectedFilePath != parentNode->FullPath)
                    {
                        // hancle click action
                        /*g_FileHandler->m_bFileSelected = false;
                        g_FileHandler->m_bFileSelected = true;*/
                        g_FileHandler->m_selectedFilePath = parentNode->FullPath;
                        printf("Clicked file: %s\n", g_FileHandler->m_selectedFilePath.c_str());
                        GetFileInformation(g_FileHandler->m_selectedFilePath);
                    }
                }
            }
            ImGui::PopID();
        }
    }

    void RenderTree()
    {
        if(g_FileHandler->m_bFileLoaded)
            DisplayDirectoryNode(g_FileHandler->m_RootNode);
    }

    void RenderPropetries()
    {
        //DisplayPropertries();
    }

    void RenderHex()
    {
        if (g_FileHandler->m_bFileSelected)
        {
            static MemoryEditor m_MemoryEdit;
            m_MemoryEdit.DrawContents(m_selectedfileContent.data(), m_selectedFileSize);
        }
    }
};

class P3DHandler : public FileHandler {
public:
    P3D p3d;

    P3DHandler() 
    {
        g_LoadManager = new LoadManager();

        g_LoadManager->AddHandler(new TextureLoader, Texture::TEXTURE, "TEXTURE");
        g_LoadManager->AddHandler(new ShaderLoader, Shader::SHADER, "SHADER");
        g_LoadManager->AddHandler(new GeometryLoader, Geometry::MESH, "MESH");
        g_LoadManager->AddHandler(new CompositeDrawableLoader, CompositeDrawable::COMPOSITE_DRAWABLE, "COMPOSITE_DRAWABLE");
        g_LoadManager->AddHandler(new SkeletonLoader, Skeleton::SKELETON, "SKELETON");

        // Print all registered handlers
        g_LoadManager->PrintHandlers();
    }

    void PrintP3DChunk(const std::vector<P3DChunk>& chunks, int depth = 0)
    {
        for (const auto& chunk : chunks)
        {
            // Print indentation based on depth
            for (int i = 0; i < depth; ++i)
                std::wcout << L"\t";

            std::wcout << L"**** Chunk ID: " << std::hex << chunk.header.data_type << std::endl;

            // Print other chunk information
            for (int i = 0; i < depth; ++i)
                std::wcout << L"\t";

            std::wcout << L"\t chunk_size: " << std::hex << chunk.header.chunk_size << std::endl;

            for (int i = 0; i < depth; ++i)
                std::wcout << L"\t";

            std::wcout << L"\t sub_chunks_size: " << std::hex << chunk.header.sub_chunks_size << std::endl;

            for (int i = 0; i < depth; ++i)
                std::wcout << L"\t";

            std::wcout << L"\t chunk offset: " << std::dec << chunk.file_offset << std::endl;

            // Recursively print child chunks
            if (!chunk.childs.empty())
            {
                std::wcout << std::endl;
                PrintP3DChunk(chunk.childs, depth + 1);
            }
        }
    }

    void PrintP3DData()
    {
        // Print header data
        std::wcout << L"File ID: " << p3d.header.file_id << std::endl;
        std::wcout << L"File size: " << p3d.header.file_size << std::endl;
        std::wcout << L"File version: " << p3d.header.version << std::endl;

        // Print chunks recursively
        PrintP3DChunk(p3d.chunks);
    }

    void Load(const std::wstring& filePath, int offset = -1) override {
        std::wcout << L"Loading P3D file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_workingFilePath = filePath;

        FILE* m_File = nullptr;
        errno_t err = _wfopen_s(&m_File, filePath.c_str(), L"rb");
        if (err != 0 || m_File == nullptr) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        // Seek to the provided offset if provided
        if (offset != -1) {
            std::wcout << L"Seek to offset: " << offset << std::endl;
            if (fseek(m_File, offset, SEEK_SET) != 0) {
                std::wcerr << L"Error seeking to offset: " << offset << std::endl;
                fclose(m_File);
                return;
            }
        }

        // Read header
        fread(&p3d.header, sizeof(P3DHeader), 1, m_File);

        m_bFileSelected = true;
        GetFileContent(filePath, (offset != -1) ? offset : 0, p3d.header.file_size);

        if (memcmp(p3d.header.file_id, "P3D", sizeof(p3d.header.file_id)) != 0) {
            std::wcerr << L"Error: Not a valid P3D archive." << std::endl;
            fclose(m_File);
            return;
        }

        p3d.GetChunks(m_File, p3d.header.file_size + offset);

        // Close the file
        fclose(m_File);

        PrintP3DData();

        m_RootNode = new DirectoryNode();

        m_RootNode->FullPath = (offset == -1) ? std::string(filePath.begin(), filePath.end()) : std::string(m_selectedFilePath.begin(), m_selectedFilePath.end());
        m_RootNode->FileName = m_RootNode->FullPath.substr(m_RootNode->FullPath.find_last_of('\\') + 1);
        m_RootNode->IsDirectory = true;
        CreateTreeNodesFromP3DChunks(p3d.chunks, m_RootNode);
        m_bFileLoaded = true;
    }

    void CreateTreeNodesFromP3DChunks(const std::vector<P3DChunk>& chunks, DirectoryNode* parentNode) {
        for (const auto& chunk : chunks) {
            // Create file or directory node for the chunk
            DirectoryNode node;

            // Convert data_type to hexadecimal string and add chunk name
            std::stringstream ss;
            ss << std::hex << chunk.header.data_type << " - " << g_LoadManager->GetName(chunk.header.data_type);

            node.FullPath = ss.str();
            node.FileName = ss.str();
            node.file_offset = chunk.file_offset;
            node.IsDirectory = (chunk.childs.size() > 0);

            // Add the node to the parent directory
            parentNode->Children.push_back(node);

            // If the chunk has child chunks, recursively call the function
            if (chunk.childs.size() > 0) {
                CreateTreeNodesFromP3DChunks(chunk.childs, &parentNode->Children.back());
            }
        }
    }
    
    void DisplayDirectoryNode(DirectoryNode& parentNode)
    {
        if (&parentNode != nullptr)
        {
            ImGuiTreeNodeFlags nodeFlags = 0;

            if (parentNode.IsSelected)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;

            ImGui::PushID(&parentNode);

            if (parentNode.IsDirectory)
            {
                if (ImGui::TreeNodeEx(parentNode.FileName.c_str(),
                    ImGuiTreeNodeFlags_SpanFullWidth |
                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    nodeFlags))
                {

                    for (DirectoryNode& childNode : parentNode.Children)
                        DisplayDirectoryNode(childNode);
                    ImGui::TreePop();
                }
                // Handle selection logic
                if (ImGui::IsItemClicked())
                {
                    // Unselect all nodes first
                    UnselectAllNodes(*m_RootNode);
                    // Select the clicked node
                    parentNode.IsSelected = true;
                }
            }
            else
            {
                if (ImGui::TreeNodeEx(parentNode.FileName.c_str(),
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_SpanFullWidth |
                    nodeFlags))
                {
                    if (ImGui::IsItemClicked(0) && g_FileHandler->m_selectedFilePath != parentNode.FullPath)
                    {
                        // Unselect all nodes first
                        UnselectAllNodes(*m_RootNode);
                        // Select the clicked node
                        parentNode.IsSelected = true;

                        // handle click action
                        g_FileHandler->m_bFileSelected = false;
                        g_FileHandler->m_selectedFilePath = parentNode.FullPath;
                        printf("Clicked file: %s %lld\n", g_FileHandler->m_selectedFilePath.c_str(), static_cast<long long>(parentNode.file_offset));
                        g_FileHandler->m_bFileSelected = true;
                    }
                }
            }
            ImGui::PopID();
        }
    }

    void UnselectAllNodes(DirectoryNode& parentNode)
    {
        parentNode.IsSelected = false;
        for (DirectoryNode& childNode : parentNode.Children)
            UnselectAllNodes(childNode);
    }

    void RenderTree() 
    {
        if (g_FileHandler->m_bFileLoaded)
            DisplayDirectoryNode(*g_FileHandler->m_RootNode);
    }

    void RenderPropetries() { }

    void RenderHex() 
    {
        static MemoryEditor m_MemoryEdit;
        m_MemoryEdit.DrawContents(m_selectedfileContent.data(), m_selectedFileSize);
    }
};

class CSOHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath, int offset = -1)  override {
        std::wcout << "Loading CSO file: " << filePath << std::endl;
        // Add CSO file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

class BIKHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath, int offset = -1)  override {
        std::wcout << "Loading BIK file: " << filePath << std::endl;
        // Add BIK file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

class RSDHandler : public FileHandler {
public:
    void Load(const std::wstring& filePath, int offset = -1)  override {
        std::wcout << "Loading RSD file: " << filePath << std::endl;
        // Add RSD file loading logic here
    }

    void RenderTree() { }

    void RenderPropetries() { }

    void RenderHex() { }
};

// Function to open a file dialog and return the selected file path
std::string OpenFileDlg()
{
    console.log("OpenFileDlg()");
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

std::wstring GetFileExtension(const std::wstring& filePath) {
    size_t dotPos = filePath.find_last_of(L'.');
    if (dotPos != std::wstring::npos) {
        return filePath.substr(dotPos + 1);
    }
    return L""; // If no extension found
}

void ProcessFile(const std::wstring filePath, int offset = -1)
{
    console.log((offset == -1) ? "ProcessFile() disk file!" : "ProcessFile() sub file!");
    std::wstring extension = L"";
    std::string m_selectedFilePath = "";
    std::wcout << "offset: " << offset << std::endl;

    if (offset != -1)
    {
        m_selectedFilePath = g_FileHandler->m_selectedFilePath;
        extension = GetFileExtension(g_FileHandler->StringToWideString(m_selectedFilePath));
    }
    else
    {
        extension = GetFileExtension(filePath);
    }
    FileHandler::eFileType type = FileHandler::GetFileType(extension);

    std::wcout << "File extension: " << extension << std::endl;
    switch (type) {
    case FileHandler::eFileType::RCF_FILE:
        std::wcout << L"File type is RCF_FILE" << std::endl;
        g_FileHandler = std::make_unique<RCFHandler>();
        break;
    case FileHandler::eFileType::P3D_FILE:
        std::wcout << L"File type is P3D_FILE" << std::endl;
        g_FileHandler = std::make_unique<P3DHandler>();
        break;
    case FileHandler::eFileType::RSD_FILE:
        std::wcout << L"File type is RSD_FILE" << std::endl;
        g_FileHandler = std::make_unique<RSDHandler>();
        break;
    case FileHandler::eFileType::CSO_FILE:
        std::wcout << L"File type is CSO_FILE" << std::endl;
        g_FileHandler = std::make_unique<CSOHandler>();
        break;
    case FileHandler::eFileType::BIK_FILE:
        std::wcout << L"File type is BIK_FILE" << std::endl;
        g_FileHandler = std::make_unique<BIKHandler>();
        break;
    case FileHandler::eFileType::FSC_FILE:
        std::wcout << L"File type is FSC_FILE" << std::endl;
        break;
    case FileHandler::eFileType::UNK_FILE:
        std::wcout << L"Unknown file type" << std::endl;
        break;
    }

    if (g_FileHandler) {
        g_FileHandler->m_selectedFilePath = m_selectedFilePath;
        g_FileHandler->Load(filePath, offset);
    }
}

void RenderTree()
{
    if (g_FileHandler == nullptr) return;
    if (g_FileHandler->m_bFileLoaded)
    {
        g_FileHandler->RenderTree();
    }
}

void RenderPropetries()
{
    if (g_FileHandler == nullptr) return;
    if (g_FileHandler->m_bFileLoaded && g_FileHandler->m_bFileSelected)
    {
        g_FileHandler->RenderPropetries();
    }
}

void RenderHex()
{
    if (g_FileHandler == nullptr) return;
    if (g_FileHandler->m_bFileLoaded && g_FileHandler->m_bFileSelected)
    {
        g_FileHandler->RenderHex();
    }
}