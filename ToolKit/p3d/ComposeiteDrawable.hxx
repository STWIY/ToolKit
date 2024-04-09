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
	CompositeDrawable* compositedrawable = nullptr;
	void LoadObject(ChunkFile* f) override
	{
		compositedrawable = LoadCompositeDrawable(f);
		if (compositedrawable) {
			printf("Loaded compositedrawable %s\n");
		}
	}

	CompositeDrawable* LoadCompositeDrawable(ChunkFile* f)
	{
		compositedrawable = nullptr;

		return compositedrawable;
	}

	void RenderObject(int type) override
	{
		if (compositedrawable == nullptr) return;
		ImGui::Begin("Compositedrawable Information");

		ImGui::End();
	}
};