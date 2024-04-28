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

	bool Initialize(float scale);
	void Render();

private:
	void CreateBox(float scale);

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};