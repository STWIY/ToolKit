#pragma once

#include <cstdio>
#include "P3D.h"

class Skeleton
{
public:
	enum {
		SKELETON = 0x23000
	};
};

class SkeletonLoader : public ObjectLoader
{
	Skeleton* skeleton = nullptr;
	void LoadObject(ChunkFile* f) override
	{
		skeleton = LoadSkeleton(f);
		if (skeleton) {
			printf("Loaded skeleton %s\n");
		}
	}

	Skeleton* LoadSkeleton(ChunkFile* f)
	{
		skeleton = nullptr;


		return skeleton;
	}

	void RenderObject() override
	{
		if (skeleton == nullptr) return;
		ImGui::Begin("Skeleton Information");

		ImGui::End();
	}
};