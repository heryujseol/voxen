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
	inline bool IsEmpty() { return IsEmptyLowLod(); }

	inline Vector3 GetPosition() { return m_position; }
	inline void SetPosition(Vector3 position) { m_position = position; }

	inline bool IsEmptyLowLod() { return m_lowLodVertices.empty(); }
	inline bool IsEmptyOpaque() { return m_opaqueVertices.empty(); }
	inline bool IsEmptyTransparency() { return m_transparencyVertices.empty(); }
	inline bool IsEmptySemiAlpha() { return m_semiAlphaVertices.empty(); }

	inline const std::vector<VoxelVertex>& GetLowLodVertices() const { return m_lowLodVertices; }
	inline const std::vector<uint32_t>& GetLowLodIndices() const { return m_lowLodIndices; }

	inline const std::vector<VoxelVertex>& GetOpaqueVertices() const { return m_opaqueVertices; }
	inline const std::vector<uint32_t>& GetOpaqueIndices() const { return m_opaqueIndices; }

	inline const std::vector<VoxelVertex>& GetTransparencyVertices() const
	{
		return m_transparencyVertices;
	}
	inline const std::vector<uint32_t>& GetTransparencyIndices() const
	{
		return m_transparencyIndices;
	}

	inline const std::vector<VoxelVertex>& GetSemiAlphaVertices() const
	{
		return m_semiAlphaVertices;
	}
	inline const std::vector<uint32_t>& GetSemiAlphaIndices() const { return m_semiAlphaIndices; }

	inline const std::unordered_map<uint8_t, std::vector<Vector3>>& GetInstanceMap() const
	{
		return m_instanceMap;
	}

	inline const ChunkConstantData& GetConstantData() const { return m_constantData; }


private:
	void InitChunkData();
	void InitInstanceInfoData();
	void InitWorldVerticesData();

	void MakeFaceSliceColumnBit(uint64_t cullColBit[CHUNK_SIZE_P2 * 6],
		uint64_t sliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6]);
	void GreedyMeshing(uint64_t faceColBit[CHUNK_SIZE2 * 6], std::vector<VoxelVertex>& vertices,
		std::vector<uint32_t>& indices, uint8_t type);

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	UINT m_id;
	bool m_isLoaded;
	Vector3 m_position;

	std::vector<VoxelVertex> m_lowLodVertices;
	std::vector<uint32_t> m_lowLodIndices;

	std::vector<VoxelVertex> m_opaqueVertices;
	std::vector<uint32_t> m_opaqueIndices;

	std::vector<VoxelVertex> m_transparencyVertices;
	std::vector<uint32_t> m_transparencyIndices;

	std::vector<VoxelVertex> m_semiAlphaVertices;
	std::vector<uint32_t> m_semiAlphaIndices;

	ChunkConstantData m_constantData;

	std::unordered_map<uint8_t, std::vector<Vector3>> m_instanceMap;
};