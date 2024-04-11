#ifndef P3D_HANDLER_H
#define P3D_HANDLER_H

#include "../FileHandler.hxx"

#include "pure3d/LoadManager.hxx"

#include "pure3d/Texture.hxx"
#include "pure3d/Shader.hxx"
#include "pure3d/Geometry.hxx"
#include "pure3d/ComposeiteDrawable.hxx"
#include "pure3d/Skeleton.hxx"

#include "P3D.h"

ObjectLoader* loader;

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

    void LoadFile(std::string& filePath, int offset) override
    {
        std::cout << L"Loading P3D file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_LoadedFilePath = filePath;

        m_LoadedFileName = ExtractFileNameWithoutExtension(m_LoadedFilePath);

        FILE* m_File = nullptr;
        errno_t err = fopen_s(&m_File, filePath.c_str(), "rb");
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
            ChunkNode* node = FindParentNode(topParent);
            if (node->chunk.uniqueID > 0)
            {
                // Apply the content for save the temp file
                GetFileContent(g_FileHandler->m_LoadedFilePath, node->file_offset, node->chunk.header.sub_chunks_size);
                SaveToTempFile("chunk.p3d", node->chunk.header.sub_chunks_size);
                // Apply the content for the hex viewer
                GetFileContent(g_FileHandler->m_LoadedFilePath, chunkNode.file_offset, chunkNode.chunk.header.sub_chunks_size);
                P3DChunk* chunk = p3d.GetChunkByID(&p3d.chunks, topParent);
                if (chunk != nullptr)
                {
                    if (m_selectedfileContent.size() == 0) return;
                    std::string path = std::string(m_savedFilePath.begin(), m_savedFilePath.end());
                    LoadStream* stream = new LoadStream(path.c_str());
                    ChunkFile cf(stream, true);
                    loader = g_LoadManager->GetHandler(chunk->header.data_type);
                    if (loader)
                    {
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
                if (loader) {
                    loader->RenderObject(m_selectedChunkNode->chunk.header.data_type);
                }
            }
            break;
            case eDisplayMode::VALUES:
            {
                ImGui::Text("VALUES- %d", m_selectedChunkNode->chunk.header.data_type);
                if (loader) {
                    loader->RenderObject(m_selectedChunkNode->chunk.header.data_type);
                }
            }
            break;
            case eDisplayMode::HEX:
            {
                ImGui::Text("HEX-");
                RenderHex();
            }
            break;
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
                g_FileHandler->ProcessFile(filePath);
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

#endif // P3D_HANDLER_H