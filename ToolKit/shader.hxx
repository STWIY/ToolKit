#pragma once

#include <cstdio>
#include "P3D.h"

class Shader
{
public:
	enum {
		SHADER = 0x11000
	};
};

class ShaderLoader : public ObjectLoader
{
	void LoadObject(FILE* m_File) override
	{
		printf("ShaderLoader::LoadObject()\n");
		Shader* shader = LoadShader(m_File);
		if (shader) {
			printf("Loaded shader %s\n");
		}
	}

	Shader* LoadShader(FILE* m_File)
	{
		Shader* shader = nullptr;


		return shader;
	}
};