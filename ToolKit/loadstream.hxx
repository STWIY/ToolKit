#pragma once
#include "core.hxx"

#include <stdio.h>
#include <assert.h>

class LoadStream
{
	FILE *fp;
public:
	LoadStream(const char* filename)
	{
		fp = nullptr;
		OpenRead(filename);
	}

	~LoadStream(void)
	{
		Close();
	}

	bool OpenRead(const char* filename)
	{

		fp = fopen(filename, "rb");
		return fp != nullptr;
	}

	void Close(void)
	{
		if (fp)
			fclose(fp);
	}

	bool GetData(void* buf, u32 count, u32 sz = 1)
	{
		return fread(buf, sz, count, fp) == count;
	}

	u32 GetSize(void)
	{
		assert(0);
		return 0;
	}

	u32 GetPosition(void)
	{
		return ftell(fp);
	}

	void Advance(u32 skip)
	{
		fseek(fp, skip, SEEK_CUR);
	}

	bool IsOpen(void) { return fp != nullptr; }

	u8 GetU8(void) { u8 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	u16 GetU16(void) { u16 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	u32 GetU32(void) { u32 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	i8 GetI8(void) { i8 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	i16 GetI16(void) { i16 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	i32 GetI32(void) { i32 tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	float GetFloat(void) { float tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
};
