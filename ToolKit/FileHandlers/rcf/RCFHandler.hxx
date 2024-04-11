#ifndef RCF_HANDLER_H
#define RCF_HANDLER_H

#include "../FileHandler.hxx"

#include "RCF.h"

class RCFHandler : public FileHandler
{
public:
    RCFHandler() {}
    RCF rcf;

    struct FileNode
    {
        std::string FullPath;
        std::string FileName;
        std::vector<FileNode> Children;
        bool IsDirectory;
        FileHandler::eFileType FileType;
        RCFFilenameDirectoryEntry* entry = nullptr;
        RCFDirectoryEntry* dir_entry = nullptr;
    };

    FileNode* m_RootNode;
    FileNode* m_NodeSelected;

    void LoadFile(std::string& filePath, int offset) override
    {
        std::cout << L"Loading RCF file: " << filePath << std::endl;

        m_bFileLoaded = false;

        if (offset == -1) m_LoadedFilePath = filePath;

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

        m_RootNode = new FileNode();

        m_RootNode->FullPath = (offset == -1) ? std::string(filePath.begin(), filePath.end()) : std::string(m_selectedFilePath.begin(), m_selectedFilePath.end());
        m_RootNode->FileName = m_RootNode->FullPath.substr(m_RootNode->FullPath.find_last_of('\\') + 1);
        m_RootNode->IsDirectory = true;
        CreateTreeNodesFromPaths(m_RootNode);
        m_bFileLoaded = true;
    }

    void CreateTreeNodesFromPaths(FileNode* parentNode)
    {
        for (auto& filenameEntry : rcf.filename_directory) {
            std::string directoryPath = filenameEntry.path.substr(0, filenameEntry.path.find_last_of('\\'));

            std::string fileName = g_FileHandler->ExtractFileName(filenameEntry.path);

            std::istringstream iss(directoryPath);
            std::string directory;
            FileNode* currentNode = parentNode;
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
                        FileNode newNode;
                        newNode.FileName = directory;
                        newNode.FullPath = directory;
                        newNode.FileType = eFileType::DIR_FILE;
                        newNode.IsDirectory = true;
                        currentNode->Children.push_back(newNode);
                        currentNode = &currentNode->Children.back();
                    }
                }
            }

            FileNode fileNode;
            fileNode.FileName = ExtractFileName(filenameEntry.path);
            fileNode.FullPath = filenameEntry.path;
            fileNode.FileType = GetFileTypeFromExtension(GetFileExtension(filenameEntry.path));
            fileNode.IsDirectory = false;
            fileNode.entry = &filenameEntry;
            currentNode->Children.push_back(fileNode);
        }
    }

    std::string TimestampToString(uint32_t timestamp) {
        time_t rawtime = static_cast<time_t>(timestamp);

        struct tm* timeinfo;
        timeinfo = localtime(&rawtime);

        char buffer[80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

        return std::string(buffer);
    }

    bool GetFileInformation(std::string path)
    {
        int index = 0;
        for (auto& filenameEntry : rcf.filename_directory) {
            if (filenameEntry.path == path)
            {
                printf("File index: %d\n", index);
                printf("File path: %s\n", path.c_str());
                printf("File offset: %d\n", rcf.directory[index].fl_offset);
                printf("File size: %d\n", rcf.directory[index].fl_size);
                printf("File hash: %p\n", rcf.directory[index].hash);
                std::wstring wPath(path.begin(), path.end());
                GetFileContent(g_FileHandler->m_LoadedFilePath, rcf.directory[index].fl_offset, rcf.directory[index].fl_size);
                return true;
            }
            index++;
        }

        return false;
    }

    void DisplayDirectoryNode(FileNode* parentNode)
    {
        if (parentNode != nullptr)
        {
            ImGui::PushID(parentNode);

            ImGuiTreeNodeFlags m_TreeNodeFlags = IMGUI_TREENODE_FLAGS;
            if (m_NodeSelected == parentNode)
                m_TreeNodeFlags |= ImGuiTreeNodeFlags_Selected;

            if (parentNode->IsDirectory)
            {
                if (ImGui::TreeNodeEx(parentNode->FileName.c_str(), m_TreeNodeFlags))
                {
                    for (FileNode& childNode : parentNode->Children)
                        DisplayDirectoryNode(&childNode);
                    ImGui::TreePop();
                }
            }
            else
            {
                if (ImGui::TreeNodeEx(parentNode->FileName.c_str(), m_TreeNodeFlags | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
                {
                    if (ImGui::IsItemClicked(0))
                    {
                        g_FileHandler->m_selectedFilePath = parentNode->FullPath;
                        printf("Clicked file: %s\n", g_FileHandler->m_selectedFilePath.c_str());
                        GetFileInformation(g_FileHandler->m_selectedFilePath);
                    }
                }

                if (ImGui_ToolTipHover())
                {
                    parentNode->dir_entry = GetDirectoryEntry(rcf, parentNode->FullPath);
                    std::pair<const char*, std::string> m_ResourceInfoList[] =
                    {
                        { "Name",          parentNode->FileName },
                        { "Path",          parentNode->FullPath },
                        { "Type",          GetFileExtensionFromType(parentNode->FileType) },
                        { "Date",          (parentNode->entry) ? TimestampToString(parentNode->entry->date) : "-" },
                        { "Size",          (parentNode->dir_entry) ? std::to_string(parentNode->dir_entry->fl_size) : "-" },
                    };

                    // Loop through the m_ResourceInfoList array
                    for (auto& m_Pair : m_ResourceInfoList)
                    {
                        ImGui::Text("%s:", m_Pair.first);
                        ImGui::SameLine(80.f);
                        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_COLOR_TEXT2);
                        ImGui::Text(&m_Pair.second[0]);
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndTooltip();
                }
            }

            // Mark node as selected
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                m_NodeSelected = parentNode;

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
        if (m_NodeSelected)
        {
            if (m_NodeSelected->entry != nullptr)
            {
                ImGui::Text(m_NodeSelected->entry->path.c_str());
            }
            if (m_NodeSelected->dir_entry != nullptr)
            {
                ImGui::Text("%d", m_NodeSelected->dir_entry->fl_size);
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

        ImGui::Begin(g_HexEditorTitle);
        {
            RenderHex();
        }
        ImGui::End();
    }
};

#endif // RCF_HANDLER_H