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

	enum 
	{
		TEXTURE = 0x19000,
		IMAGE = 0x19001,
		IMAGE_DATA = 0x19002,
		IMAGE_FILENAME = 0x19003,
		VOLUME_IMAGE = 0x19004
	};
};

class TextureLoader : public ObjectLoader
{
public:
	Texture* texture;
	void LoadObject(ChunkFile* f) override
	{
		texture = LoadTexture(f);
		if (texture) {
			printf("Loaded texture %s\n", texture->name);
		}
	}

	Texture* LoadTexture(ChunkFile* f)
	{
		Texture* texture = new Texture();
		char name[128];
		f->GetString(texture->name);
		texture->version = f->GetI32();
		texture->width = f->GetI32();
		texture->height = f->GetI32();
		texture->bpp = f->GetI32();
		texture->alphaDepth = f->GetI32();
		texture->numMipmaps = f->GetI32();
		if (texture->numMipmaps) texture->numMipmaps--;
		// pddi stuff
		u32 textureType = f->GetU32();
		std::cout << "\t textureType: " << std::dec << textureType << std::endl;
		u32 usage = f->GetU32();
		std::cout << "\t usage: " << std::dec << usage << std::endl;

		printf("reading texture %s %d %d %d\n", texture->name, texture->width, texture->height, texture->bpp);

			// TODO: imageFactory stuff

		int mipmap = 0;
		bool volume = false;
		bool image = false;
		/*while (f->ChunksRemaining() && mipmap <= numMipmaps) {
			switch (f->BeginChunk()) {
			case Texture::IMAGE:
				if (!volume) {
					texture = LoadImage(f, texture, mipmap);
					volume = false;
					image = true;
					mipmap++;
				}
				break;
			case Texture::VOLUME_IMAGE:
				if (!image) {
					volume = true;
					image = false;
					mipmap++;
				}
				break;
			}
			f->EndChunk();
		}*/

		return texture;
	}

	Texture* LoadImage(ChunkFile* f, Texture* buildTexture, int mipmap)
	{
		// TODO: imageFactory stuff
		char name[128];
		f->GetString(name);
		i32 version = f->GetI32();
		i32 width = f->GetI32();
		i32 height = f->GetI32();
		i32 bpp = f->GetI32();
		i32 palettized = f->GetI32();
		i32 alpha = f->GetI32();
		u32 format = f->GetU32();

		printf("reading image %s %d %d %d %X\n", name, width, height, bpp, format);

		Texture* texture = buildTexture;
		while (f->ChunksRemaining()) {
			switch (f->BeginChunk()) {
			case Texture::IMAGE_DATA: {
				u32 sz = f->GetU32();
				LoadStream* s = f->BeginInset();
				/*if (texture)
					imageFactory->ParseIntoTexture(s, texture, sz, format, mipmap);
				else
					texture = imageFactory->ParseAsTexture(s, sz, format);*/
				f->EndInset(s);
				break;
			}

			case Texture::IMAGE_FILENAME: {
				char fileName[256];
				f->GetString(fileName);
				assert(0 && "cannot load images\n");
				break;
			}

			}
			f->EndChunk();
		}

		return texture;
	}

	void RenderObject() override
	{
		if (texture == nullptr) return;
		ImGui::Text("Name: %s", texture->name);
		ImGui::Text("Version: %u", texture->version);
		ImGui::Text("Width: %u", texture->width);
		ImGui::Text("Height: %u", texture->height);
		ImGui::Text("Bits Per Pixel: %u", texture->bpp);
	}
};