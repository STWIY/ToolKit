#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

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
	std::streampos file_offset;
};

class ObjectLoader {
public:
	virtual void LoadObject(FILE* m_File) = 0;
};

class LoadManager {
	std::map<uint32_t, std::pair<ObjectLoader*, std::string>> objectHandlers;

public:
	void AddHandler(ObjectLoader* loader, uint32_t chunkId, std::string name) {
		objectHandlers[chunkId] = std::make_pair(loader, name);
	}

	ObjectLoader* GetHandler(uint32_t chunkId) {
		auto it = objectHandlers.find(chunkId);
		if (it != objectHandlers.end()) {
			return it->second.first;
		}
		return nullptr;
	}

	std::string GetName(uint32_t chunkId) {
		auto it = objectHandlers.find(chunkId);
		if (it != objectHandlers.end()) {
			return it->second.second;
		}
		return "";
	}

	void PrintHandlers() {
		std::cout << "Registered Handlers:" << std::endl;
		for (const auto& pair : objectHandlers) {
			std::cout << std::hex << "Chunk ID: " << pair.first << ", Name: " << pair.second.second << ", Handler Address: " << pair.second.first << std::endl;
		}
	}
};

LoadManager* g_LoadManager;

class P3D {
public:
	P3DHeader header;
	std::vector<P3DChunk> chunks;

	P3DChunk ReadChunk(FILE* m_File) {
		P3DChunk chunk;
		chunk.file_offset = std::ftell(m_File);
		fread(&chunk.header, sizeof(chunk.header), 1, m_File);
		chunk.body.resize(chunk.header.chunk_size - sizeof(chunk.header));
		fread(chunk.body.data(), chunk.body.size(), 1, m_File);

		// Handle chunk loading using LoadManager
		ObjectLoader* loader = g_LoadManager->GetHandler(chunk.header.data_type);
		if (loader) {
			//loader->LoadObject(m_File);
		}

		return chunk;
	}

	void GetChunks(FILE* m_File, std::streampos section_end) {
		while (ftell(m_File) < section_end || section_end == std::streampos(0)) {
			auto& chunk = chunks.emplace_back(ReadChunk(m_File));
			auto header = chunk.header;
			if (header.chunk_size != header.sub_chunks_size) {
				auto chunk_left = header.sub_chunks_size - header.chunk_size;
				while (chunk_left) {
					auto next = ReadChunk(m_File);
					chunk_left -= next.header.chunk_size;
					chunk.childs.emplace_back(next);
					if (next.header.chunk_size == next.header.sub_chunks_size)
						continue;
					else {
						auto child_left = next.header.sub_chunks_size - next.header.chunk_size;
						while (child_left) {
							next = ReadChunk(m_File);
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