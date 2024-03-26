#pragma once

#include <cstdio>
#include "P3D.h"

class CompositeDrawable
{
public:
	enum {
		COMPOSITE_DRAWABLE = 0x123000
	};
};

class CompositeDrawableLoader : public ObjectLoader
{
	void LoadObject(FILE* m_File) override
	{
		CompositeDrawable* compositedrawable = LoadCompositeDrawable(m_File);
		if (compositedrawable) {
			printf("Loaded compositedrawable %s\n");
		}
	}

	CompositeDrawable* LoadCompositeDrawable(FILE* m_File)
	{
		CompositeDrawable* compositedrawable = nullptr;

		return compositedrawable;
	}
};