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
	static const int CHUNK_SIZE = 32;
	static const int CHUNK_SIZE2 = CHUNK_SIZE * CHUNK_SIZE;
	static const int CHUNK_SIZE_P = CHUNK_SIZE + 2;
	static const int CHUNK_SIZE_P2 = CHUNK_SIZE_P * CHUNK_SIZE_P;

	Chunk(UINT id);
	~Chunk();

	void Initialize();
	void Update(float dt);
	void Clear();

	inline UINT GetID() { return m_id; }

	inline void SetLoad(bool isLoaded) { m_isLoaded = isLoaded; }
	inline bool IsLoaded() { return m_isLoaded; }
	inline bool IsEmpty() { return IsEmptyBasic() && IsEmptyWater() && IsEmptySprite(); }
	
	inline Vector3 GetPosition() { return m_position; }
	inline void SetPosition(Vector3 position) { m_position = position; }

	inline bool IsEmptyBasic() { return m_basicVertices.empty(); }
	inline bool IsEmptyWater() { return m_waterVertices.empty(); }
	inline bool IsEmptySprite() { return m_spriteVertices.empty(); }

	inline const std::vector<VoxelVertex>& GetBasicVertices() const { return m_basicVertices; }
	inline const std::vector<uint32_t>& GetBasicIndices() const { return m_basicIndices; }

	inline const std::vector<VoxelVertex>& GetWaterVertices() const { return m_waterVertices; }
	inline const std::vector<uint32_t>& GetWaterIndices() const { return m_waterIndices; }

	inline const std::vector<VoxelVertex>& GetSpriteVertices() const { return m_spriteVertices; }

	inline const ChunkConstantData& GetConstantData() const { return m_constantData; }
	

private:
	void InitChunkData();
	void InitSpriteVerticesData();
	void InitMeshVerticesData();

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	UINT m_id;
	bool m_isLoaded;
	Vector3 m_position;

	std::vector<VoxelVertex> m_basicVertices;
	std::vector<uint32_t> m_basicIndices;

	std::vector<VoxelVertex> m_waterVertices;
	std::vector<uint32_t> m_waterIndices;

	std::vector<VoxelVertex> m_spriteVertices;

	ChunkConstantData m_constantData;
};