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
	
	Chunk(int x, int y, int z);
	~Chunk();

	bool Initialize();
	void Update(float dt);
	void Render();
	
	bool IsLoaded();
	bool IsEmpty();
	Vector3 GetPosition();

	static const int BLOCK_SIZE = 32;

private:
	Chunk();
	void CreateBlock(
		int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p);

	bool m_isLoaded;
	UINT m_stride;
	UINT m_offset;

	Vector3 m_position;
	std::vector<std::vector<std::vector<Block>>> m_blocks;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	size_t m_indexCount;
	ChunkConstantData m_constantData;

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;

	std::vector<std::tuple<int, int, int>> m_activeBlocks;
};