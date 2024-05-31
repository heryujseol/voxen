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

	static const int CHUNK_COUNT = 7;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_COUNT = MAX_HEIGHT / Chunk::CHUNK_SIZE;
	static const int CHUNK_COUNT_P = CHUNK_COUNT + 2;
	static const int MAX_HEIGHT_CHUNK_COUNT_P = MAX_HEIGHT_CHUNK_COUNT + 2;
	static const int MAX_ASYNC_LOAD_COUNT = 1;

private:
	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunks();
	void UpdateUnloadChunks();
	
	bool FrustumCulling(Vector3 position, Camera& camera);

	Chunk* GetChunkFromPool();
	void ReleaseChunkToPool(Chunk* chunk);

	std::vector<Chunk*> m_chunkPool;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunkMap;
	std::vector<Chunk*> m_loadChunkList;
	std::vector<Chunk*> m_unloadChunkList;
};