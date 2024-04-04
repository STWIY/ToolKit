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
	Shader* shader = nullptr;
	void LoadObject(ChunkFile* f) override
	{
		shader = LoadShader(f);
		if (shader) {
			printf("Loaded shader %s\n");
		}
	}

	Shader* LoadShader(ChunkFile* f)
	{
		shader = nullptr;


		return shader;
	}

	void RenderObject() override
	{
		if (shader == nullptr) return;
		ImGui::Begin("Shader Information");

		ImGui::End();
	}
};