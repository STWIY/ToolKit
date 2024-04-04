#include <vector>
#include <iostream>

#include "chunkfile.hxx"

class ObjectLoader
{
public:
	virtual void LoadObject(ChunkFile* f) = 0;
	virtual void RenderObject() = 0;
};

class LoadManager
{
	std::map<uint32_t, std::pair<ObjectLoader*, std::string>> objectHandlers;
public:
	void AddHandler(ObjectLoader* loader, uint32_t chunkId, std::string name)
	{
		objectHandlers[chunkId] = std::make_pair(loader, name);
	}

	ObjectLoader* GetHandler(uint32_t chunkId)
	{
		auto it = objectHandlers.find(chunkId);
		if (it != objectHandlers.end()) {
			return it->second.first;
		}
		return nullptr;
	}

	std::string GetName(uint32_t chunkId)
	{
		auto it = objectHandlers.find(chunkId);
		if (it != objectHandlers.end()) {
			return it->second.second;
		}
		return "";
	}
};

LoadManager* g_LoadManager;