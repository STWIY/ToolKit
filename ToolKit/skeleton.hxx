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
	void LoadObject(FILE* m_File) override
	{
		Skeleton* skeleton = LoadSkeleton(m_File);
		if (skeleton) {
			printf("Loaded skeleton %s\n");
		}
	}

	Skeleton* LoadSkeleton(FILE* m_File)
	{
		Skeleton* skeleton = nullptr;


		return skeleton;
	}
};