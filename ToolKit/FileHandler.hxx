#pragma once

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cerrno>

#include "Console.hxx"
#include "p3d/LoadManager.hxx"

#include "RCF.h"
#include "p3d/P3D.h"

// 3rdParty (ImGui)
#include "3rdParty/ImGui/imgui.h"
#include "3rdParty/ImGui/imgui_internal.h"
#include "3rdParty/ImGui/imgui_impl_win32.h"
#include "3rdParty/ImGui/imgui_impl_dx11.h"
#include "3rdParty/ImGui/imgui_memory_editor.h"

#include "p3d/Texture.hxx"
#include "p3d/Shader.hxx"
#include "p3d/Geometry.hxx"
#include "p3d/ComposeiteDrawable.hxx"
#include "p3d/Skeleton.hxx"

ObjectLoader* loader;

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
        UNK_FILE
    };

    // Loaded file from disc
    eFileType m_LoadedFileType = eFileType::UNK_FILE;

    // Loaded file path from disc
    std::wstring m_LoadedFilePath;

    // Loaded file name
    std::string m_LoadedFileName;

    // Identifier if file has been loaded
    bool m_bFileLoaded = false;

    // Selected file from the Tree nodes
    std::string m_selectedFilePath;

    // Binary content of the selected file
    std::vector<uint8_t> m_selectedfileContent;
    int m_selectedFileSize;

    std::wstring m_savedFilePath;

    virtual void LoadFile(const std::wstring& filePath, int offset = -1) = 0;

    virtual void Render() = 0;

    // Extract file name from string path
    std::string ExtractFileName(const std::string& filePath)
    {
        size_t found = filePath.find_last_of("/\\");
        if (found != std::string::npos) {
            return filePath.substr(found + 1);
        }
        return filePath;
    }

    std::string ExtractFileNameWithoutExtension(const std::string& filePath)
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
    std::wstring GetFileExtension(const std::wstring& filePath) {
        size_t dotPos = filePath.find_last_of(L'.');
        if (dotPos != std::wstring::npos) {
            return filePath.substr(dotPos + 1);
        }
        return L""; // If no extension found
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

        m_savedFilePath = tempFilePath; // Assign the correct path
    }

    // Gets from disc file using its path, offset and size the binary content of the file
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

    // Default function used for each different handler
    void ProcessFile(const std::wstring filePath, int offset = -1);
};

std::unique_ptr<FileHandler> g_FileHandler;


class RCFHandler : public FileHandler 
{
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
                //SaveToTempFile()
                //ProcessFile(g_FileHandler->m_LoadedFilePath);// , rcf.directory[index].fl_offset);
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

class P3DHandler : public FileHandler 
{
public:
    P3D p3d;

    enum eDisplayMode
    {
        DEFAULT,
        VALUES,
        HEX
    };

    struct ChunkNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<ChunkNode> Children;
        bool IsDirectory;
        int file_size;
        std::streampos file_offset;
        P3DChunk chunk;
        bool IsSelected;
        eDisplayMode displayMode = eDisplayMode::DEFAULT;
        uint64_t parentID;
    };

    ChunkNode* m_RootNode;
    ChunkNode* m_selectedChunkNode;

    P3DHandler()
    {
        g_LoadManager = new LoadManager();

        g_LoadManager->AddHandler(new TextureLoader, Texture::TEXTURE, "TEXTURE");
        g_LoadManager->AddHandler(new ShaderLoader, Shader::SHADER, "SHADER");
        g_LoadManager->AddHandler(new GeometryLoader, Geometry::MESH, "MESH");
        g_LoadManager->AddHandler(new CompositeDrawableLoader, CompositeDrawable::COMPOSITE_DRAWABLE, "COMPOSITE_DRAWABLE");
        g_LoadManager->AddHandler(new SkeletonLoader, Skeleton::SKELETON, "SKELETON");
    }

    void LoadFile(const std::wstring& filePath, int offset = -1)
    {
        std::wcout << L"Loading P3D file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_LoadedFilePath = filePath;
        
        m_LoadedFileName = ExtractFileNameWithoutExtension(std::string(m_LoadedFilePath.begin(), m_LoadedFilePath.end()));

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

        //GetFileContent(filePath, (offset != -1) ? offset : 0, p3d.header.file_size);

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

        //p3d.LoadFile(std::string(filePath.begin(), filePath.end()).c_str());
    }

    void CreateTreeNodesFromP3DChunks(const std::vector<P3DChunk>& chunks, ChunkNode* parentNode) 
    {
        for (P3DChunk chunk : chunks) {
            // Create file or directory node for the chunk
            ChunkNode node;

            // Convert data_type to hexadecimal string and add chunk name
            std::stringstream ss;
            ss << "(" << std::dec << chunk.uniqueID << ")" << std::hex << chunk.header.data_type << " - " << g_LoadManager->GetName(chunk.header.data_type);
            node.FullPath = ss.str();
            node.FileName = ss.str();
            node.file_offset = chunk.file_offset;
            node.IsDirectory = (chunk.childs.size() > 0);
            node.chunk = chunk;
            node.parentID = parentNode->chunk.uniqueID;
            // Add the node to the parent directory
            parentNode->Children.push_back(node);

            // If the chunk has child chunks, recursively call the function
            if (chunk.childs.size() > 0) {
                CreateTreeNodesFromP3DChunks(chunk.childs, &parentNode->Children.back());
            }
            else {
                node.parentID = NULL;
            }
        }
    }

    uint64_t FindTopParentID(ChunkNode& node)
    {
        if (node.parentID == 0) {
            // This is the top parent node
            return node.chunk.uniqueID;
        }
        else {
            // Recursively call the function with the parent node
            ChunkNode* parentNode = FindParentNode(node.parentID);
            if (parentNode != nullptr) {
                return FindTopParentID(*parentNode);
            }
            else {
                // If parent node is not found, return the current node's unique ID
                return node.chunk.uniqueID;
            }
        }
    }

    ChunkNode* FindParentNode(uint64_t parentID)
    {
        // Traverse the tree to find the node with the given parentID
        return FindParentNodeRecursive(m_RootNode, parentID);
    }

    ChunkNode* FindParentNodeRecursive(ChunkNode* currentNode, uint64_t parentID)
    {
        if (currentNode->chunk.uniqueID == parentID) {
            // Found the node with the given parentID
            return currentNode;
        }

        // Recursively search in the children nodes
        for (auto& child : currentNode->Children) {
            ChunkNode* foundNode = FindParentNodeRecursive(&child, parentID);
            if (foundNode != nullptr) {
                return foundNode;
            }
        }

        // If not found in children, return nullptr
        return nullptr;
    }

    void LoadChunkContent(ChunkNode chunkNode)
    {
        uint64_t topParent = FindTopParentID(chunkNode);
        if (topParent)
        {
            printf("***** Top parent id: %d\n", topParent);
            ChunkNode* node = FindParentNode(topParent);
            if (node->chunk.uniqueID>0)
            {
                printf("\t - Node id: %d\n", node->chunk.header.data_type);
                GetFileContent(g_FileHandler->m_LoadedFilePath, node->file_offset, node->chunk.header.sub_chunks_size);
                SaveToTempFile("chunk.p3d", node->chunk.header.sub_chunks_size);
                P3DChunk* chunk = p3d.GetChunkByID(&p3d.chunks, topParent);
                if (chunk != nullptr)
                {
                    if (m_selectedfileContent.size() == 0) return;
                    std::string path = std::string(m_savedFilePath.begin(), m_savedFilePath.end());
                    LoadStream* stream = new LoadStream(path.c_str());
                    ChunkFile cf(stream, true);
                    loader = g_LoadManager->GetHandler(chunk->header.data_type);
                    if (loader) {
                        printf("***** Loading object id: %d - and render props of %d\n", chunk->header.data_type, chunkNode.chunk.uniqueID);
                        loader->LoadObject(&cf);
                    }
                    stream->Close();
                }
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
                    //GetFileContent(g_FileHandler->m_LoadedFilePath, chunkNode.file_offset, chunkNode.chunk.header.sub_chunks_size);
                    //SaveToTempFile("chunk.p3d", chunkNode.chunk.header.sub_chunks_size);
                    LoadChunkContent(chunkNode);
                    m_selectedChunkNode = &chunkNode;
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
                    //GetFileContent(g_FileHandler->m_LoadedFilePath, chunkNode.file_offset, chunkNode.chunk.header.sub_chunks_size);
                    //SaveToTempFile("chunk.p3d", chunkNode.chunk.header.sub_chunks_size);
                    LoadChunkContent(chunkNode);
                    m_selectedChunkNode = &chunkNode;
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

    void DisplayModeSelector(eDisplayMode& currentMode) {
        const char* items[] = { 
            "Default", 
            "Values", 
            "Hex" 
        };
        int currentItem = static_cast<int>(currentMode);
        if (ImGui::BeginCombo("Display Mode", items[currentItem])) {
            for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                const bool isSelected = (currentItem == i);
                if (ImGui::Selectable(items[i], isSelected))
                    currentMode = static_cast<eDisplayMode>(i);

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void RenderPropetries()
    {
        if (m_selectedChunkNode != nullptr)
        {
            DisplayModeSelector(m_selectedChunkNode->displayMode);
            switch (m_selectedChunkNode->displayMode)
            {
                case eDisplayMode::DEFAULT:
                {
                    ImGui::Text("DEFAULT-");
                    break;
                }
                case eDisplayMode::VALUES:
                {
                    ImGui::Text("VALUES- %d", m_selectedChunkNode->chunk.header.data_type);
                    if (loader) {
                        loader->RenderObject(m_selectedChunkNode->chunk.header.data_type);
                    }
                    break;
                }
                case eDisplayMode::HEX:
                {
                    ImGui::Text("HEX-");
                    RenderHex();
                    break;
                }
            }
        }
    }

    void RenderHex()
    {
        if (m_selectedfileContent.size() > 0)
        {
            static MemoryEditor m_MemoryEdit;
            m_MemoryEdit.DrawContents(m_selectedfileContent.data(), m_selectedFileSize);
        }
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

class CSOHandler : public FileHandler 
{
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