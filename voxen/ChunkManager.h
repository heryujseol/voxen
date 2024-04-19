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

	void Initialize(ComPtr<ID3D11Device>& device, Vector3 cameraOffset);
	void Update(ComPtr<ID3D11Device>& device, Camera& camera);
	void Render(ComPtr<ID3D11DeviceContext>& m_context, Camera& camera);

	void LoadChunks(ComPtr<ID3D11Device>& device);
	void UnloadChunks();
	void UpdateChunkList(Vector3 cameraOffset);

	bool FrustumCulling(Vector3 position, Camera& camera);

private:
	static const int CHUNK_SIZE = 11;
	std::map<std::tuple<int, int, int>, Chunk> m_chunks;

	std::vector<Vector3> m_loadChunkList;
	std::vector<Vector3> m_unloadChunkList;

	std::future<void> m_loadFuture;
};