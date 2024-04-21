#pragma once

#include <map>
#include <vector>
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

	bool Initialize(Vector3 cameraOffset);
	void Update(Camera& camera);
	void Render(Camera& camera);

private:
	void LoadChunks();
	void UnloadChunks();
	void UpdateChunkList(Vector3 cameraOffset);
	bool FrustumCulling(Vector3 position, Camera& camera);

	static const int CHUNK_SIZE = 7;
	std::map<std::tuple<int, int, int>, Chunk> m_chunks;

	std::vector<Vector3> m_loadChunkList;
	std::vector<Vector3> m_unloadChunkList;

	std::future<void> m_loadFuture;
};