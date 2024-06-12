#pragma once

#include <map>
#include <unordered_map>
#include <queue>
#include <future>

#include "Chunk.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class ChunkManager {

public:
	static const int CHUNK_COUNT = 7;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_COUNT = 8;
	static const int CHUNK_COUNT_P = CHUNK_COUNT + 2;
	static const int MAX_HEIGHT_CHUNK_COUNT_P = MAX_HEIGHT_CHUNK_COUNT + 2;
	static const int MAX_ASYNC_LOAD_COUNT = 1;
	static const int MAX_INSTANCE_RENDER_DISTANCE = 160;
	static const int MAX_INSTANCE_BUFFER_SIZE = 1024 * 1024 * 8;
	static const int MAX_INSTANCE_BUFFER_COUNT = MAX_INSTANCE_BUFFER_SIZE / sizeof(InstanceInfo);

	ChunkManager();
	~ChunkManager();

	bool Initialize(Vector3 cameraChunkPos);
	void Update(Camera& camera);

	void RenderOpaque();
	void RenderSemiAlpha();
	void RenderTransparency();

	void RenderInstance();

private:
	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunkList();
	void UpdateUnloadChunkList();
	void UpdateRenderChunkList(Camera& camera);
	void UpdateInstanceInfoList(Camera& camera);

	bool FrustumCulling(Vector3 position, Camera& camera);

	bool MakeBuffer(Chunk* chunk);
	void ClearChunkBuffer(Chunk* chunk);
	Chunk* GetChunkFromPool();
	void ReleaseChunkToPool(Chunk* chunk);

	bool MakeInstanceVertexBuffer();
	bool MakeInstanceInfoBuffer();

	std::vector<Chunk*> m_chunkPool;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunkMap;

	std::vector<Chunk*> m_loadChunkList;
	std::vector<Chunk*> m_unloadChunkList;
	std::vector<Chunk*> m_renderChunkList;

	std::vector<ComPtr<ID3D11Buffer>> m_opaqueVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_opaqueIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_transparencyVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_transparencyIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_semiAlphaVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_semiAlphaIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_instanceVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_instanceIndexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_instanceInfoBuffers;
	std::vector<std::vector<InstanceInfo>> m_instanceInfoList;
	std::vector<UINT> m_instanceIndexCount;
};