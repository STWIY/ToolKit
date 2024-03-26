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
	void LoadObject(FILE* m_File) override
	{
		Geometry* geometry = LoadGeometry(m_File);
		if (geometry) {
			printf("Loaded geometry %s\n");
		}
	}

	Geometry* LoadGeometry(FILE* m_File)
	{
		Geometry* geometry = nullptr;


		return geometry;
	}
};