#pragma once

#include <cstdio>
#include "P3D.h"

class Geometry
{
public:
	enum {
		MESH = 0x10000
	};
};

class GeometryLoader : public ObjectLoader
{
	Geometry* geometry = nullptr;
	void LoadObject(ChunkFile* f) override
	{
		geometry = LoadGeometry(f);
		if (geometry) {
			printf("Loaded geometry %s\n");
		}
	}

	Geometry* LoadGeometry(ChunkFile* f)
	{
		geometry = nullptr;


		return geometry;
	}

	void RenderObject() override
	{
		if (geometry == nullptr) return;
		ImGui::Begin("Geometry Information");

		ImGui::End();
	}
};