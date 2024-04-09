#pragma once

#include "Block.h"
#include "Utils.h"
#include <iostream>

class Chunk {

public:
	Chunk();
	Chunk(int x, int y, int z);
	~Chunk();

	bool Initialize(ComPtr<ID3D11Device>& device);
	void Update(ComPtr<ID3D11DeviceContext>& context, float dt);
	void Render(ComPtr<ID3D11DeviceContext>& context);

	bool IsEmpty();
	Vector3 GetPosition();

	static const int BLOCK_SIZE = 32;

private:
	void CreateBlock(
		int x, int y, int z, bool x_n, bool x_p, bool y_n, bool y_p, bool z_n, bool z_p);

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
};