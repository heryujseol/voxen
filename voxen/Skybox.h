#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl.h>

#include "Structure.h"
#include "Utils.h"

using namespace Microsoft::WRL;

class Skybox 
{
public:
	Skybox();
	~Skybox();

	bool Initialize();
	void Render();

private:
	void CreateBox(float scale);

	UINT m_stride;
	UINT m_offset;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	size_t m_indexCount;

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};