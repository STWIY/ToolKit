#pragma once

#include <cstdint>
#include <vector>

#pragma pack(push, 1)

struct P3DHeader {
    char file_id[3];
    char _pad;
    uint32_t version;
    uint32_t file_size;
};

struct P3DChunkHeader {
    uint32_t data_type;
    uint32_t chunk_size;
    uint32_t sub_chunks_size;
};

#pragma pack(pop)

struct P3DChunk {
    P3DChunkHeader header;
    std::vector<uint8_t> body;
    std::vector<P3DChunk> childs;
};

struct P3D {
    P3DHeader header;
    std::vector<P3DChunk> chunks;

    P3DChunk read_chunk(std::ifstream& file)
    {
        P3DChunk chunk;
        file.read(reinterpret_cast<char*>(&chunk.header), sizeof(chunk.header));
        chunk.body.resize(chunk.header.chunk_size - sizeof(chunk.header));
        file.read(reinterpret_cast<char*>(chunk.body.data()), chunk.body.size());
        return chunk;
    }

    void get_chunks(std::ifstream& file, std::streampos section_end)
    {
        while (file.tellg() < section_end || section_end == std::streampos(0)) {
            auto& chunk = chunks.emplace_back(read_chunk(file));
            auto header = chunk.header;
            if (header.chunk_size != header.sub_chunks_size) {
                auto chunk_left = header.sub_chunks_size - header.chunk_size;
                while (chunk_left) {
                    auto next = read_chunk(file);
                    chunk_left -= next.header.chunk_size;
                    chunk.childs.emplace_back(next);
                    if (next.header.chunk_size == next.header.sub_chunks_size)
                        continue;
                    else {
                        auto child_left = next.header.sub_chunks_size - next.header.chunk_size;
                        while (child_left) {
                            next = read_chunk(file);
                            child_left -= next.header.chunk_size;
                            chunk_left -= next.header.chunk_size;
                            chunk.childs.back().childs.emplace_back(next);
                        }
                    }
                }
            }
        }
    }
};