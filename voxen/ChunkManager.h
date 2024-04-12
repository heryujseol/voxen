#pragma once

#include <map>
#include <vector>
#include <thread>
#include <future>

#include "Chunk.h"
#include "Camera.h"

class ChunkManager {

public:
	ChunkManager();
	~ChunkManager();

	void Initialize(ComPtr<ID3D11Device>& device, Vector3 cameraOffset);
	void Update(ComPtr<ID3D11Device>& device, Camera& camera);
	void Render(ComPtr<ID3D11DeviceContext>& m_context);

	void LoadChunks(ComPtr<ID3D11Device>& device);
	void UnloadChunks();
	void UpdateChunkList(Vector3 cameraOffset);

private:
	static const int CHUNK_SIZE = 7;
	std::map<std::tuple<int, int, int>, Chunk> m_chunks;

	std::vector<Vector3> m_loadChunkList;
	std::vector<Vector3> m_unloadChunkList;

	std::future<void> m_loadFuture;
};