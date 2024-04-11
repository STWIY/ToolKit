#pragma once

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

	bool GetData(void* buf, uint32_t count, uint32_t sz = 1)
	{
		return fread(buf, sz, count, fp) == count;
	}

	uint32_t GetSize(void)
	{
		assert(0);
		return 0;
	}

	uint32_t GetPosition(void)
	{
		return ftell(fp);
	}

	void Advance(uint32_t skip)
	{
		fseek(fp, skip, SEEK_CUR);
	}

	bool IsOpen(void) { return fp != nullptr; }

	uint8_t GetU8(void) { uint8_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	uint16_t GetU16(void) { uint16_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	uint32_t GetU32(void) { uint32_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	int8_t GetI8(void) { int8_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	int16_t GetI16(void) { int16_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	int32_t GetI32(void) { int32_t tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
	float GetFloat(void) { float tmp; GetData(&tmp, 1, sizeof(tmp)); return tmp; }
};
