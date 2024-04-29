#pragma once

#include <map>
#include <queue>
#include <future>

#include "Chunk.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class ChunkManager {

public:
	ChunkManager();
	~ChunkManager();

	bool Initialize(Vector3 cameraChunkPos);
	void Update(Camera& camera);
	void Render(Camera& camera);

private:
	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunks();
	void UpdateUnloadChunks();
	
	bool FrustumCulling(Vector3 position, Camera& camera);

	Chunk* GetChunkFromPool();
	void ReleaseChunkToPool(Chunk* chunk);

	static const int CHUNK_SIZE = 11;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_SIZE = MAX_HEIGHT / Chunk::BLOCK_SIZE;
	static const int MAX_ASYNC_LOAD_COUNT = 4;

	std::vector<Chunk*> m_chunkPool;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunkMap;
	std::vector<Chunk*> m_loadChunkList;
	std::vector<Chunk*> m_unloadChunkList;

	std::vector<std::future<bool>> m_loadFutures;
};