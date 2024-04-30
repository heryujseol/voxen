#pragma once

#include "Block.h"
#include "Structure.h"

#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <directxtk/SimpleMath.h>

using namespace Microsoft::WRL; 
using namespace DirectX::SimpleMath;


class Chunk {

public:
	Chunk();
	~Chunk();

	bool Initialize();
	void Update(float dt);
	void Render();
	void Clear();

	inline bool IsLoaded() { return m_isLoaded; }
	inline bool IsEmpty() { return m_vertices.empty(); }

	inline Vector3 GetPosition() { return m_position; }
	inline void SetPosition(Vector3 position) { m_position = position; }

private:
	void CreateQuad(int x, int y, int z, int merged, int length, int face, int type);
	VoxelVertex MakeVertex(int x, int y, int z, int face, int type);

	bool m_isLoaded;

	Vector3 m_position;

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	std::vector<VoxelVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	ChunkConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};