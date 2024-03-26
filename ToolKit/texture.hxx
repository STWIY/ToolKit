#pragma once

#include <cstdio>

#include "P3D.h"

class Texture
{
public:
	char name[128];
	uint32_t version;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	uint32_t alphaDepth;
	uint32_t numMipmaps;

	enum {
		TEXTURE = 0x19000,
		IMAGE = 0x19001,
		IMAGE_DATA = 0x19002,
		IMAGE_FILENAME = 0x19003,
		VOLUME_IMAGE = 0x19004
	};
};

class TextureLoader : public ObjectLoader
{
	void LoadObject(FILE* m_File) override
	{
		printf("TextureLoader::LoadObject()\n");
		Texture* texture = LoadTexture(m_File);
		if (texture) {
			printf("Loaded texture %s\n");
		}
	}

	Texture* LoadTexture(FILE* m_File)
	{
		Texture* texture = nullptr;
		fread(&texture->name, sizeof(char), 128, m_File);
		fread(&texture->version, sizeof(uint32_t), 1, m_File);
		fread(&texture->width, sizeof(uint32_t), 1, m_File);
		fread(&texture->height, sizeof(uint32_t), 1, m_File);
		fread(&texture->bpp, sizeof(uint32_t), 1, m_File);
		fread(&texture->alphaDepth, sizeof(uint32_t), 1, m_File);
		fread(&texture->numMipmaps, sizeof(uint32_t), 1, m_File);
		
		printf("texture %s %d %d %d\n", texture->name, texture->width, texture->height, texture->bpp);

		// TODO:Texture::IMAGE
		// TODO:Texture::VOLUME_IMAGE

		return texture;
	}

	Texture* LoadImage(std::ifstream& file, Texture* texture, int mipmap)
	{
		// TODO:
		Texture* texture = nullptr;

		return texture;
	}
};