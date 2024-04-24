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

	bool IsLoaded();
	bool IsEmpty();

	Vector3 GetPosition();
	void SetPosition(Vector3 position);

	static const int BLOCK_SIZE = 32;

private:
	void CreateBlock(
		int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p);
	
	UINT m_stride;
	UINT m_offset;

	bool m_isLoaded;
	Vector3 m_position;

	Block m_blocks[BLOCK_SIZE][BLOCK_SIZE][BLOCK_SIZE];

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	size_t m_indexCount;
	ChunkConstantData m_constantData;

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};