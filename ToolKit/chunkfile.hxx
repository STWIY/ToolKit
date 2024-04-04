#pragma once
#include "core.hxx"
#include "loadstream.hxx"

#include <string.h>
#include <assert.h>

struct Chunk
{
	u32 id, dataLength, chunkLength, startPosition;
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
		u32 id = realFile->GetU32();
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

	u32 BeginChunk(void)
	{
		stackTop++;

		// skip end to end of chunk
		if (stackTop != 0) {
			u32 start = chunkStack[stackTop - 1].startPosition + chunkStack[stackTop - 1].dataLength;
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

	u32 BeginChunk(u32 chunkID)
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
		u32 start = chunkStack[stackTop].startPosition;
		u32 chunkLength = chunkStack[stackTop].chunkLength;
		realFile->Advance(start + chunkLength - realFile->GetPosition());
		stackTop--;
	}

	u32 GetCurrentID(void)
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

	void GetData(void *buf, u32 count, u32 sz = 1) { realFile->GetData(buf, count, sz); }
	u8 GetU8(void) { return realFile->GetU8(); }
	u16 GetU16(void) { return realFile->GetU16(); }
	u32 GetU32(void) { return realFile->GetU32(); }
	i8 GetI8(void) { return realFile->GetI8(); }
	i16 GetI16(void) { return realFile->GetI16(); }
	i32 GetI32(void) { return realFile->GetI32(); }
	float GetFloat(void) { return realFile->GetFloat(); }
	char *GetString(char *s) { u8 tmp = GetU8(); realFile->GetData(s, tmp); s[tmp] = '\0'; return s; }
	char *GetLString(char *s) { u32 tmp = GetU32(); realFile->GetData(s, tmp+1); return s; }
};
