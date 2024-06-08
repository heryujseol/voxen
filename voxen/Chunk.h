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

	inline bool IsEmptyBasic() { return m_basicVertice.empty(); }
	inline bool IsEmptyWater() { return m_waterVertice.empty(); }
	inline bool IsEmptySprite() { return m_spriteVertice.empty(); }

	inline const std::vector<VoxelVertex>& GetBasicVertice() const { return m_basicVertice; }
	inline const std::vector<uint32_t>& GetBasicIndice() const { return m_basicIndice; }

	inline const std::vector<VoxelVertex>& GetWaterVertice() const { return m_waterVertice; }
	inline const std::vector<uint32_t>& GetWaterIndice() const { return m_waterIndice; }

	inline const std::vector<VoxelVertex>& GetSpriteVertice() const { return m_spriteVertice; }

	inline const ChunkConstantData& GetConstantData() const { return m_constantData; }
	

private:
	void InitChunkData();
	void InitSpriteVerticeData();
	void InitMeshVerticeData();
	void CreateQuad(int x, int y, int z, int merged, int length, int face, int type);
	VoxelVertex MakeVertex(int x, int y, int z, int face, int type);

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	UINT m_id;
	bool m_isLoaded;
	Vector3 m_position;

	std::vector<VoxelVertex> m_basicVertice;
	std::vector<uint32_t> m_basicIndice;

	std::vector<VoxelVertex> m_waterVertice;
	std::vector<uint32_t> m_waterIndice;

	std::vector<VoxelVertex> m_spriteVertice;

	ChunkConstantData m_constantData;
};