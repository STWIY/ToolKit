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

    eFileType m_LoadedFileType = eFileType::UNK_FILE;
    std::wstring m_LoadedFilePath;
    bool m_bFileLoaded = false;

    std::string m_selectedFilePath;

    std::vector<char> m_selectedfileContent;
    int m_selectedFileSize;

    virtual void LoadFile(const std::wstring& filePath, int offset = -1) = 0;

    virtual void Render() = 0;

    std::string ExtractFileName(const std::string& filePath)
    {
        size_t found = filePath.find_last_of("/\\");
        if (found != std::string::npos) {
            return filePath.substr(found + 1);
        }
        return filePath;
    }

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

    void SaveToTempFile(std::string fileName, int size)
    {
        // Create a temporary file
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        std::filesystem::path tempFilePath = tempDir / fileName;
        FILE* tempFile = nullptr;
        errno_t err = fopen_s(&tempFile, tempFilePath.string().c_str(), "wb");
        if (err != 0 || tempFile == nullptr) {
            std::cerr << "Failed to create temporary file!" << std::endl;
            return;
        }

        // Write the content to the temporary file
        fwrite(m_selectedfileContent.data(), size, 1, tempFile);

        // Close the temporary file
        fclose(tempFile);

        m_LoadedFilePath = tempDir / fileName;

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

        SaveToTempFile(ExtractFileName(m_selectedFilePath).c_str(), size);
    }

    void ProcessFile(const std::wstring filePath, int offset = -1);
};

std::unique_ptr<FileHandler> g_FileHandler;


class RCFHandler : public FileHandler {
public:
    RCF rcf;

    struct DirectoryNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<DirectoryNode> Children;
        bool IsDirectory;
        std::streampos file_offset;
        bool IsSelected;
    };

    DirectoryNode* m_RootNode;

    void LoadFile(const std::wstring& filePath, int offset = -1)
    {
        std::wcout << L"Loading RCF file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_LoadedFilePath = filePath;

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

    void CreateTreeNodesFromPaths(DirectoryNode* parentNode) 
    {
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

                GetFileContent(g_FileHandler->m_LoadedFilePath, rcf.directory[index].fl_offset, rcf.directory[index].fl_size);

                ProcessFile(g_FileHandler->m_LoadedFilePath);// , rcf.directory[index].fl_offset);
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
        if (g_FileHandler->m_bFileLoaded)
            DisplayDirectoryNode(m_RootNode);
    }

    void RenderPropetries()
    {
    }

    void RenderHex()
    {
    }

    void Base()
    {
        ImGui::SetNextWindowPos({ 0.f, 0.f });
        ImGui::SetNextWindowSize(g_ImGuiIO->DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });

        ImGui::Begin("##Base", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

        ImGui::PopStyleVar(3);

        ImGuiID m_DockSpaceID = ImGui::GetID("##BaseDockSpace");
        ImGui::DockSpace(m_DockSpaceID, { 0.f, 0.f }, ImGuiDockNodeFlags_PassthruCentralNode);

        static bool m_BuildDockSpace = true;
        if (m_BuildDockSpace)
        {
            m_BuildDockSpace = false;
            {
                ImGui::DockBuilderDockWindow(g_TreeTitle, ImGui::DockBuilderSplitNode(m_DockSpaceID, ImGuiDir_Left, 0.45f, nullptr, nullptr));

                ImGuiID m_PropertiesID = ImGui::DockBuilderSplitNode(m_DockSpaceID, ImGuiDir_None, 0.f, nullptr, nullptr);
                ImGui::DockBuilderDockWindow(g_PropertiesTitle, m_PropertiesID);
                ImGui::DockBuilderDockWindow(g_HexEditorTitle, ImGui::DockBuilderSplitNode(m_PropertiesID, ImGuiDir_Down, 0.45f, nullptr, nullptr));
            }
            ImGui::DockBuilderFinish(m_DockSpaceID);
        }

        bool m_OpenFile = false, m_SaveFile = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(u8"\uF15B File"))
            {
                if (ImGui::MenuItemEx("Open", u8"\uF56F", "CTRL + O"))
                    m_OpenFile = true;


                ImGui::EndMenu();
            }
        }

        // Shortcuts
        if (ImGui::GetCurrentContext()->OpenPopupStack.empty())
        {
            std::pair<std::pair<ImGuiKey, ImGuiKey>, bool*> m_Shortcuts[] =
            {
                { { ImGuiKey_LeftCtrl, ImGuiKey_O }, &m_OpenFile },
                { { ImGuiKey_LeftCtrl, ImGuiKey_S }, &m_SaveFile },
            };
            for (auto& m_Pair : m_Shortcuts)
            {
                ImGuiKey m_Key = m_Pair.first.second;
                if (m_Key == ImGuiKey_None)
                    m_Key = m_Pair.first.first;
                else if (!ImGui::IsKeyDown(m_Pair.first.first))
                    continue;

                if (!ImGui::IsKeyPressed(m_Key, false))
                    continue;

                *m_Pair.second = true;
            }
        }

        // Options
        if (m_OpenFile)
        {
            // Example usage
            std::string filePath = OpenFileDlg();
            if (!filePath.empty()) {
                // File selected, process it
                g_FileHandler->ProcessFile(std::wstring(filePath.begin(), filePath.end()));
            }
            else {
                // No file selected or dialog canceled
                std::cout << "No file selected or dialog canceled." << std::endl;
            }
        }

        ImGui::End();
    }

    void Render()
    {
        Base();
        ImGui::Begin(g_TreeTitle);
        {
            RenderTree();
        }
        ImGui::End();

        ImGui::Begin(g_PropertiesTitle);
        {
            RenderPropetries();
        }
        ImGui::End();

        ImGui::Begin(g_HexEditorTitle);
        {
            RenderHex();
        }
        ImGui::End();
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

    struct ChunkNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<ChunkNode> Children;
        bool IsDirectory;
        int file_size;
        std::streampos file_offset;
        P3DChunk* chunk;
        bool IsSelected;
    };

    ChunkNode* m_RootNode;
    P3DChunk* m_selectedChunkNode;

    void LoadFile(const std::wstring& filePath, int offset = -1)
    {
        std::wcout << L"Loading P3D file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_LoadedFilePath = filePath;

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

        GetFileContent(filePath, (offset != -1) ? offset : 0, p3d.header.file_size);

        if (memcmp(p3d.header.file_id, "P3D", sizeof(p3d.header.file_id)) != 0) {
            std::wcerr << L"Error: Not a valid P3D archive." << std::endl;
            fclose(m_File);
            return;
        }

        p3d.GetChunks(m_File, p3d.header.file_size + offset);

        // Close the file
        fclose(m_File);

        m_RootNode = new ChunkNode();

        m_RootNode->FullPath = (offset == -1) ? std::string(filePath.begin(), filePath.end()) : std::string(m_selectedFilePath.begin(), m_selectedFilePath.end());
        m_RootNode->FileName = m_RootNode->FullPath.substr(m_RootNode->FullPath.find_last_of('\\') + 1);
        m_RootNode->IsDirectory = true;
        CreateTreeNodesFromP3DChunks(p3d.chunks, m_RootNode);
        m_bFileLoaded = true;
    }

    void CreateTreeNodesFromP3DChunks(const std::vector<P3DChunk>& chunks, ChunkNode* parentNode) 
    {
        for (P3DChunk chunk : chunks) {
            // Create file or directory node for the chunk
            ChunkNode node;

            // Convert data_type to hexadecimal string and add chunk name
            std::stringstream ss;
            ss << std::hex << chunk.header.data_type << " - " << g_LoadManager->GetName(chunk.header.data_type);

            node.FullPath = ss.str();
            node.FileName = ss.str();
            node.file_offset = chunk.file_offset;
            node.IsDirectory = (chunk.childs.size() > 0);
            node.chunk = &chunk;

            // Add the node to the parent directory
            parentNode->Children.push_back(node);

            // If the chunk has child chunks, recursively call the function
            if (chunk.childs.size() > 0) {
                CreateTreeNodesFromP3DChunks(chunk.childs, &parentNode->Children.back());
            }
        }
    }
    
    void DisplayDirectoryNode(ChunkNode& chunkNode)
    {
        ImGui::PushID(&chunkNode);

        ImGuiTreeNodeFlags nodeFlags = 0;

        if (chunkNode.IsSelected)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;

        if (chunkNode.IsDirectory)
        {
            if (ImGui::TreeNodeEx(chunkNode.FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | nodeFlags))
            {
                if (ImGui::IsItemClicked(0))
                {
                    // Unselect all nodes first
                    UnselectAllNodes(*m_RootNode);
                    // Select the clicked node
                    chunkNode.IsSelected = true;

                    g_FileHandler->m_selectedFilePath = chunkNode.FullPath;
                    printf("Clicked file: %s %lld\n", g_FileHandler->m_selectedFilePath.c_str(), static_cast<long long>(chunkNode.file_offset));
                    GetFileContent(g_FileHandler->m_LoadedFilePath, chunkNode.file_offset, chunkNode.file_size);
                }
                for (ChunkNode& childNode : chunkNode.Children)
                    DisplayDirectoryNode(childNode); // Pass by reference here
                ImGui::TreePop();
            }
        }
        else
        {
            if (ImGui::TreeNodeEx(chunkNode.FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth | nodeFlags))
            {

                if (ImGui::IsItemClicked(0))
                {
                    // Unselect all nodes first
                    UnselectAllNodes(*m_RootNode);
                    // Select the clicked node
                    chunkNode.IsSelected = true;

                    g_FileHandler->m_selectedFilePath = chunkNode.FullPath;
                    printf("Clicked file: %s %lld\n", g_FileHandler->m_selectedFilePath.c_str(), static_cast<long long>(chunkNode.file_offset));
                    GetFileContent(g_FileHandler->m_LoadedFilePath, chunkNode.file_offset, chunkNode.file_size);
                    m_selectedChunkNode = chunkNode.chunk;
                }
            }
        }
        ImGui::PopID();
    }

    void UnselectAllNodes(ChunkNode& parentNode)
    {
        parentNode.IsSelected = false;
        for (ChunkNode& childNode : parentNode.Children)
            UnselectAllNodes(childNode);
    }

    void RenderTree()
    {
        if (g_FileHandler->m_bFileLoaded)
            DisplayDirectoryNode(*m_RootNode);
    }

    void RenderPropetries()
    {        
        ObjectLoader* loader = g_LoadManager->GetHandler(m_selectedChunkNode->header.data_type);
        if (loader) {
            loader->LoadObject();
        }
        else {
            // Handle case where no loader is found for the given chunk ID
        }        
    }

    void RenderHex()
    {
    }

    void Base()
    {
        ImGui::SetNextWindowPos({ 0.f, 0.f });
        ImGui::SetNextWindowSize(g_ImGuiIO->DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });

        ImGui::Begin("##Base", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

        ImGui::PopStyleVar(2);

        ImGuiID m_DockSpaceID = ImGui::GetID("##BaseDockSpace");
        ImGui::DockSpace(m_DockSpaceID, { 0.f, 0.f }, ImGuiDockNodeFlags_PassthruCentralNode);

        static bool m_BuildDockSpace = true;
        if (m_BuildDockSpace)
        {
            m_BuildDockSpace = false;
            {
                ImGui::DockBuilderDockWindow(g_TreeTitle, ImGui::DockBuilderSplitNode(m_DockSpaceID, ImGuiDir_Left, 0.45f, nullptr, nullptr));

                ImGuiID m_PropertiesID = ImGui::DockBuilderSplitNode(m_DockSpaceID, ImGuiDir_None, 0.f, nullptr, nullptr);
                ImGui::DockBuilderDockWindow(g_PropertiesTitle, m_PropertiesID);
            }
            ImGui::DockBuilderFinish(m_DockSpaceID);
        }

        bool m_OpenFile = false, m_SaveFile = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(u8"\uF15B File"))
            {
                if (ImGui::MenuItemEx("Open", u8"\uF56F", "CTRL + O"))
                    m_OpenFile = true;


                ImGui::EndMenu();
            }
        }

        // Shortcuts
        if (ImGui::GetCurrentContext()->OpenPopupStack.empty())
        {
            std::pair<std::pair<ImGuiKey, ImGuiKey>, bool*> m_Shortcuts[] =
            {
                { { ImGuiKey_LeftCtrl, ImGuiKey_O }, &m_OpenFile },
                { { ImGuiKey_LeftCtrl, ImGuiKey_S }, &m_SaveFile },
            };
            for (auto& m_Pair : m_Shortcuts)
            {
                ImGuiKey m_Key = m_Pair.first.second;
                if (m_Key == ImGuiKey_None)
                    m_Key = m_Pair.first.first;
                else if (!ImGui::IsKeyDown(m_Pair.first.first))
                    continue;

                if (!ImGui::IsKeyPressed(m_Key, false))
                    continue;

                *m_Pair.second = true;
            }
        }

        // Options
        if (m_OpenFile)
        {
            // Example usage
            std::string filePath = OpenFileDlg();
            if (!filePath.empty()) {
                // File selected, process it
                g_FileHandler->ProcessFile(std::wstring(filePath.begin(), filePath.end()));
            }
            else {
                // No file selected or dialog canceled
                std::cout << "No file selected or dialog canceled." << std::endl;
            }
        }

        ImGui::End();
    }

    void Render()
    {
        Base();
        ImGui::Begin(g_TreeTitle);
        {
            RenderTree();
        }
        ImGui::End();

        ImGui::Begin(g_PropertiesTitle);
        {
            RenderPropetries();
        }
        ImGui::End();
    }

};

class CSOHandler : public FileHandler {
public:
    void LoadFile()
    {
        std::wcout << "Loading CSO file: " << std::endl;
        // Add CSO file loading logic here
    }

    void RenderTree()
    {
    }

    void RenderPropetries()
    {
    }

    void RenderHex()
    {
    }

    void Render()
    {
        printf("Render CSO Layout\n");
    }
};

void FileHandler::ProcessFile(const std::wstring filePath, int offset)
{
    console.log((offset == -1) ? "ProcessFile() disk file!" : "ProcessFile() sub file!");
    std::wstring extension = L"";
    std::string m_selectedFilePath = (offset != -1) ? g_FileHandler->m_selectedFilePath : "";
    std::wcout << "offset: " << offset << std::endl;

    extension = GetFileExtension(filePath);
    std::wcout << "File extension: " << extension << std::endl;

    if (extension == L"rcf")
    {
        g_FileHandler = std::make_unique<RCFHandler>();
        g_FileHandler->m_LoadedFileType = FileHandler::eFileType::CSO_FILE;
    }
    if (extension == L"p3d")
    {
        g_FileHandler = std::make_unique<P3DHandler>();
        g_FileHandler->m_LoadedFileType = FileHandler::eFileType::CSO_FILE;
    }

    if (g_FileHandler) {
        g_FileHandler->m_selectedFilePath = m_selectedFilePath;
        g_FileHandler->LoadFile(filePath, offset);
    }
}