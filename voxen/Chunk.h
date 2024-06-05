#pragma once

#include "Block.h"
#include "Structure.h"
#include "Terrain.h"

#include <d3d11.h>
#include <wrl.h>
#include <directxtk/SimpleMath.h>
#include <vector>

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

class Chunk {

public:
	Chunk();
	~Chunk();

	bool Initialize();
	void Update(float dt);

	void RenderBasic();
	void RenderSprite();
	void RenderWater();

	void Clear();

	inline bool IsLoaded() { return m_isLoaded; }
	inline bool IsEmpty() { return IsEmptyBasic() && IsEmptySprite() && IsEmptyWater(); }
	inline bool IsEmptyBasic() { return m_basicVertice.empty(); }
	inline bool IsEmptySprite() { return m_spriteVertice.empty(); }
	inline bool IsEmptyWater() { return m_waterVertice.empty(); }

	inline Vector3 GetPosition() { return m_position; }
	inline void SetPosition(Vector3 position) { m_position = position; }

	static const int CHUNK_SIZE = 32;
	static const int CHUNK_SIZE2 = CHUNK_SIZE * CHUNK_SIZE;
	static const int CHUNK_SIZE_P = CHUNK_SIZE + 2;
	static const int CHUNK_SIZE_P2 = CHUNK_SIZE_P * CHUNK_SIZE_P;

private:
	void InitChunkData();
	void InitSpriteVerticeData();
	void InitMeshVerticeData();
	void CreateQuad(int x, int y, int z, int merged, int length, int face, int type);
	VoxelVertex MakeVertex(int x, int y, int z, int face, int type);

	bool m_isLoaded;

	Vector3 m_position;

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	UINT m_stride;
	UINT m_offset;
	
	std::vector<VoxelVertex> m_basicVertice;
	std::vector<uint32_t> m_basicIndice;
	ComPtr<ID3D11Buffer> m_basicVertexBuffer;
	ComPtr<ID3D11Buffer> m_basicIndexBuffer;

	std::vector<VoxelVertex> m_spriteVertice;
	ComPtr<ID3D11Buffer> m_spriteVertexBuffer;

	std::vector<VoxelVertex> m_waterVertice;
	std::vector<uint32_t> m_waterIndice;
	ComPtr<ID3D11Buffer> m_waterVertexBuffer;
	ComPtr<ID3D11Buffer> m_waterIndexBuffer;
	
	ChunkConstantData m_constantData;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};