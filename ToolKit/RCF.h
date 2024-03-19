#pragma once
#include <cstdint>
#include <vector>
#include <string>

#pragma pack(push, 1)

// Header structure
struct RCFHeader {
    char file_id[32];
    uint32_t unk1;
    uint32_t dir_offset;
    uint32_t dir_size;
    uint32_t flnames_dir_offset;
    uint32_t flnames_dir_size;
    uint32_t unk2;
    uint32_t number_files;
};

// Directory entry structure
struct RCFDirectoryEntry {
    uint32_t hash;
    uint32_t fl_offset;
    uint32_t fl_size;
};

// Filename directory entry structure
struct RCFFilenameDirectoryEntry {
    uint32_t date;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t path_len;
    std::string path;
    uint32_t padding;
};

#pragma pack(pop)

// Main structure to hold the data for cement files
struct RCF {
    RCFHeader header;
    std::vector<RCFDirectoryEntry> directory;
    std::vector<RCFFilenameDirectoryEntry> filename_directory;
};