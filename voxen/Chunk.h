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

	bool Initialize_NO_OPT();
	bool Initialize();
	void Update(float dt);
	void Render();
	void Clear();

	inline bool IsLoaded() { return m_isLoaded; }
	inline bool IsEmpty() { return m_indexCount == 0; }

	inline Vector3 GetPosition() { return m_position; }
	inline void SetPosition(Vector3 position) { m_position = position; }

private:
	void CreateQuad(int x, int y, int z, int w, int h, int face);
	void CreateBlock(
		int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p);
	
	
	bool m_isLoaded;
	Vector3 m_position;

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	size_t m_indexCount;
	ChunkConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};