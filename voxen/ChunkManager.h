#pragma once

#include <map>
#include <queue>
#include <thread>
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
	void LoadChunks();
	void UnloadChunks();
	void UpdateChunkList(Vector3 cameraChunkPos, int moveDirX, int moveDirZ);
	bool FrustumCulling(Vector3 position, Camera& camera);

	static const int CHUNK_SIZE = 7;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_SIZE = MAX_HEIGHT / Chunk::BLOCK_SIZE;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunks;

	std::queue<Vector3> m_loadChunkList;
	std::queue<Vector3> m_unloadChunkList;

	std::future<void> m_loadFuture;
};