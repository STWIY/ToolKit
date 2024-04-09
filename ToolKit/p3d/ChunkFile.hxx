#pragma once

#include "LoadStream.hxx"

#include <string.h>
#include <assert.h>

struct Chunk
{
	uint32_t id, dataLength, chunkLength, startPosition;
};

class ChunkFile
{
	char filename[128];
	Chunk chunkStack[32];
	int stackTop;
	LoadStream *realFile;

public:

	ChunkFile(LoadStream* f, bool skip_header = false) : stackTop(-1)
	{
		realFile = f;
		uint32_t id = realFile->GetU32();
		printf("Chunk %08X\n", id);

		if (id != 0xFF443350 && !skip_header) {
			fprintf(stderr, "ERROR: not a pure3d chunk file\n");
			return;
		}
		BeginChunk(id);
	}

	bool ChunksRemaining(void)
	{
		Chunk* c = &chunkStack[stackTop];
		return c->chunkLength > c->dataLength &&
			realFile->GetPosition() < c->startPosition + c->chunkLength;
	}

	void indent(int n) { while (n--) putchar(' '); }

	uint32_t BeginChunk(void)
	{
		stackTop++;

		// skip end to end of chunk
		if (stackTop != 0) {
			uint32_t start = chunkStack[stackTop - 1].startPosition + chunkStack[stackTop - 1].dataLength;
			if (realFile->GetPosition() < start)
				realFile->Advance(start - realFile->GetPosition());
		}
		chunkStack[stackTop].startPosition = realFile->GetPosition();
		chunkStack[stackTop].id = GetU32();
		chunkStack[stackTop].dataLength = GetU32();
		chunkStack[stackTop].chunkLength = GetU32();
		//indent(stackTop);
		printf("Chunk %08X at %08X (%08X %08X)\n", chunkStack[stackTop].id, chunkStack[stackTop].startPosition, chunkStack[stackTop].dataLength, chunkStack[stackTop].chunkLength);

		return chunkStack[stackTop].id;
	}

	uint32_t BeginChunk(uint32_t chunkID)
	{
		stackTop++;
		chunkStack[stackTop].startPosition = realFile->GetPosition() - 4;
		chunkStack[stackTop].id = chunkID;
		chunkStack[stackTop].dataLength = GetU32();
		chunkStack[stackTop].chunkLength = GetU32();
		//indent(stackTop);
		printf("Chunk %08X at %08X (%08X %08X)\n", chunkStack[stackTop].id, chunkStack[stackTop].startPosition, chunkStack[stackTop].dataLength, chunkStack[stackTop].chunkLength);

		return chunkStack[stackTop].id;
	}

	void EndChunk(void)
	{
		uint32_t start = chunkStack[stackTop].startPosition;
		uint32_t chunkLength = chunkStack[stackTop].chunkLength;
		realFile->Advance(start + chunkLength - realFile->GetPosition());
		stackTop--;
	}

	uint32_t GetCurrentID(void)
	{
		return chunkStack[stackTop].id;
	}

	void SetName(const char* name)
	{
		strncpy(filename, name, sizeof(filename) - 1);
	}

	LoadStream* BeginInset(void)
	{
		return realFile;
	}

	void EndInset(LoadStream* f)
	{
		assert(f->GetPosition() <= chunkStack[stackTop].startPosition + chunkStack[stackTop].chunkLength);
	}

	const char *GetName(void) { return filename; }

	void GetData(void *buf, uint32_t count, uint32_t sz = 1) { realFile->GetData(buf, count, sz); }
	uint8_t GetU8(void) { return realFile->GetU8(); }
	uint16_t GetU16(void) { return realFile->GetU16(); }
	uint32_t GetU32(void) { return realFile->GetU32(); }
	int8_t GetI8(void) { return realFile->GetI8(); }
	int16_t GetI16(void) { return realFile->GetI16(); }
	int32_t GetI32(void) { return realFile->GetI32(); }
	float GetFloat(void) { return realFile->GetFloat(); }
	char *GetString(char *s) { uint8_t tmp = GetU8(); realFile->GetData(s, tmp); s[tmp] = '\0'; return s; }
	char *GetLString(char *s) { uint32_t tmp = GetU32(); realFile->GetData(s, tmp+1); return s; }
};
