#pragma once
#include <cstdint>
#include <vector>

#pragma pack(push, 1)

// Header structure
struct P3DHeader {
    char file_id[3];
    char _pad;
    uint32_t version;
    uint32_t file_size;
};

// Chunk header structure
struct P3DChunkHeader {
    uint32_t data_type;
    uint32_t chunk_size;
    uint32_t sub_chunk_left_length;
};

#pragma pack(pop)

// Chunk structure
struct P3DChunk {
    P3DChunkHeader chunk_header;
    uint8_t* chunk_body;
};

// Main structure to hold the data for pure3d files
struct P3D {
    P3DHeader header;
    // Dynamically allocate an array of chunks
    std::vector<P3DChunk*> chunks;
};