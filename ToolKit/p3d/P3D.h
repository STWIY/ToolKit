#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

#include "ChunkFile.hxx"

#pragma pack(push, 1)

struct P3DHeader 
{
    char file_id[3];
    char _pad;
    uint32_t version;
    uint32_t file_size;
};

struct P3DChunkHeader 
{
    uint32_t data_type;
    uint32_t chunk_size;
    uint32_t sub_chunks_size;
};

#pragma pack(pop)

struct P3DChunk 
{
	uint64_t uniqueID;
    P3DChunkHeader header;
    std::vector<uint8_t> body;
    std::vector<P3DChunk> childs;
	std::streampos file_offset;
};

class P3D 
{
public:
	P3DHeader header;
	std::vector<P3DChunk> chunks;
	uint64_t currentID = 1;

	P3DChunk ReadChunk(FILE* m_File) 
	{
		P3DChunk chunk;
		chunk.uniqueID = currentID++;
		chunk.file_offset = std::ftell(m_File);
		fread(&chunk.header, sizeof(chunk.header), 1, m_File);
		chunk.body.resize(chunk.header.chunk_size - sizeof(chunk.header));
		fread(chunk.body.data(), chunk.body.size(), 1, m_File);
		return chunk;
	}

	void GetChunks(FILE* m_File, std::streampos section_end) 
	{
		while (ftell(m_File) < section_end || section_end == std::streampos(0)) 
		{
			auto& chunk = chunks.emplace_back(ReadChunk(m_File));
			auto header = chunk.header;
			if (header.chunk_size != header.sub_chunks_size) 
			{
				auto chunk_left = header.sub_chunks_size - header.chunk_size;
				while (chunk_left) {
					auto next = ReadChunk(m_File);
					chunk_left -= next.header.chunk_size;
					chunk.childs.emplace_back(next);
					if (next.header.chunk_size == next.header.sub_chunks_size)
						continue;
					else 
					{
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

	P3DChunk* GetChunkByID(std::vector<P3DChunk>* chunks, uint64_t chunkID)
	{
		for (auto& chunk : *chunks)
		{
			if (chunk.uniqueID == chunkID)
			{
				return &chunk;
			}
			else
			{
				if (!chunk.childs.empty())
				{
					P3DChunk* foundChunk = GetChunkByID(&chunk.childs, chunkID);
					if (foundChunk != nullptr)
					{
						return foundChunk;
					}
				}
			}
		}
		return nullptr; // Chunk with specified ID not found
	}

	void LoadFile(std::string filename)
	{
		LoadStream* stream = new LoadStream(filename.c_str());
		if (stream == nullptr) {
			printf("opening file %s\n", filename.c_str());
			if (!stream->IsOpen()) {
				fprintf(stderr, "couldn't open file %s\n", filename.c_str());
				return;
			}
		}

		ChunkFile cf(stream);
		cf.SetName(filename.c_str());

		while (cf.ChunksRemaining()) {
			cf.BeginChunk();
			ObjectLoader* loader = g_LoadManager->GetHandler(cf.GetCurrentID());
			if (loader) {
				loader->LoadObject(&cf);
			}
			else
				0 && printf("no loader for chunk %X\n", cf.GetCurrentID());
			//		ReadShit(&cf);
			cf.EndChunk();
		}
	}

};