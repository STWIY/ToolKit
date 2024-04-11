#pragma once

#include <cstdio>
#include "P3D.h"
#include "lodepng/lodepng.h"

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
	uint32_t palettized;
	uint32_t alpha;
	uint32_t format;

	ID3D11ShaderResourceView* tex = NULL;

	enum 
	{
		TEXTURE = 0x19000,
		IMAGE = 0x19001,
		IMAGE_DATA = 0x19002,
		IMAGE_FILENAME = 0x19003,
		VOLUME_IMAGE = 0x19004
	};

	void AddMipmap(uint32_t format, uint32_t sz, uint8_t* data)
	{
		if (numMipmaps)
			return;
		numMipmaps++;

		if (format != 1)
			return;	// can only do PNG here

		uint8_t* image;
		uint32_t width, height;
		uint32_t error = lodepng_decode32(&image, &width, &height, data, sz);
		if (error) {
			fprintf(stderr, "PNG error\n");
			return;
		}


		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = image;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;

		g_Device->CreateTexture2D(&desc, &subResource, &pTexture);
		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		g_Device->CreateShaderResourceView(pTexture, &srvDesc, &tex);
		pTexture->Release();

		free(image);
	}
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
		char name[128];
		f->GetString(name);
		int32_t version = f->GetI32();
		int32_t width = f->GetI32();
		int32_t height = f->GetI32();
		int32_t bpp = f->GetI32();
		int32_t alphaDepth = f->GetI32();
		int32_t numMipmaps = f->GetI32();
		if (numMipmaps) numMipmaps--;
		// pddi stuff
		uint32_t textureType = f->GetU32();
		uint32_t usage = f->GetU32();

		//printf("reading texture %s %d %d %d\n", name, width, height, bpp);

		// TODO: imageFactory stuff

		texture = new Texture();
		int mipmap = 0;
		bool volume = false;
		bool image = false;
		while (f->ChunksRemaining() && mipmap <= texture->numMipmaps) {
			switch (f->BeginChunk()) {
			case Texture::IMAGE:
				//printf("Texture::IMAGE\n");
				if (!volume) {
					texture = LoadImage(f, texture, mipmap);
					volume = false;
					image = true;
					mipmap++;
				}
				break;
			case Texture::VOLUME_IMAGE:
				//printf("Texture::VOLUME_IMAGE\n");
				if (!image) {
					volume = true;
					image = false;
					mipmap++;
				}
				break;
			}
			f->EndChunk();
		}

		return texture;
	}

	Texture* LoadImage(ChunkFile* f, Texture* buildTexture, int mipmap)
	{
		// TODO: imageFactory stuff
		f->GetString(buildTexture->name);
		buildTexture->version = f->GetI32();
		buildTexture->width = f->GetI32();
		buildTexture->height = f->GetI32();
		buildTexture->bpp = f->GetI32();
		buildTexture->palettized = f->GetI32();
		buildTexture->alpha = f->GetI32();
		buildTexture->format = f->GetI32();

		//printf("reading image %s %d %d %d %X\n", buildTexture->name, buildTexture->width, buildTexture->height, buildTexture->bpp, buildTexture->format);

		while (f->ChunksRemaining()) {
			switch (f->BeginChunk()) {
			case Texture::IMAGE_DATA: {
				uint32_t sz = f->GetU32();
				LoadStream* s = f->BeginInset();

				uint8_t* data = new uint8_t[sz];
				s->GetData(data, sz);
				texture->AddMipmap(buildTexture->format, sz, data);
				delete[] data;
				
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

	void RenderObject(int type) override
	{
		if (texture == nullptr) return;
		switch (type) {
			case Texture::TEXTURE:
				RenderTextureValues();
				break;
			case Texture::IMAGE:
				RenderImageValues();
				break;
			case Texture::IMAGE_DATA:
				RenderImage();
				break;
		}
	}

	void RenderTextureValues()
	{
		ImGui::Text("Name: %s", texture->name);
		ImGui::Text("Version: %u", texture->version);
		ImGui::Text("Width: %u", texture->width);
		ImGui::Text("Height: %u", texture->height);
		ImGui::Text("Bits Per Pixel: %u", texture->bpp);
	}

	void RenderImageValues()
	{
		ImGui::Text("Name: %s", texture->name);
		ImGui::Text("Version: %u", texture->version);
		ImGui::Text("Width: %u", texture->width);
		ImGui::Text("Height: %u", texture->height);
		ImGui::Text("Bits Per Pixel: %u", texture->bpp);
		ImGui::Text("Palettized: %u", texture->palettized);
		ImGui::Text("Has alpha: %u", texture->alpha);
		ImGui::Text("Number of mipmaps: %u", texture->numMipmaps);
	}

	void RenderImage()
	{
		ImGui::Text("Image data");
		ImGui::Image((void*)texture->tex, ImVec2(texture->width, texture->height));
	}
};