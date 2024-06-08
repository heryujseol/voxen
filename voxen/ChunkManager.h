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
	static const int CHUNK_COUNT = 21;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_COUNT = 8;
	static const int CHUNK_COUNT_P = CHUNK_COUNT + 2;
	static const int MAX_HEIGHT_CHUNK_COUNT_P = MAX_HEIGHT_CHUNK_COUNT + 2;
	static const int MAX_ASYNC_LOAD_COUNT = 1;

	ChunkManager();
	~ChunkManager();

	bool Initialize(Vector3 cameraChunkPos);
	void Update(Camera& camera);
	void RenderBasic();
	void RenderSprite();
	void RenderWater();

private:
	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunks();
	void UpdateUnloadChunks();
	void UpdateRenderChunks(Camera& camera);

	bool FrustumCulling(Vector3 position, Camera& camera);

	bool MakeBuffer(Chunk* chunk);
	void ClearChunkBuffer(Chunk* chunk);
	Chunk* GetChunkFromPool();
	void ReleaseChunkToPool(Chunk* chunk);

	std::vector<Chunk*> m_chunkPool;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunkMap;

	std::vector<Chunk*> m_loadChunkList;
	std::vector<Chunk*> m_unloadChunkList;
	std::vector<Chunk*> m_renderChunkList;

	UINT m_stride;
	UINT m_offset;

	std::vector<ComPtr<ID3D11Buffer>> m_basicVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_basicIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_waterVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_waterIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_spriteInstanceBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;


};