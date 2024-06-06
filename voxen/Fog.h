#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl.h>

#include "Structure.h"
#include "Utils.h"

using namespace Microsoft::WRL;

class Fog {
public:
	Fog();
	~Fog();

	bool Initialize();
	//void Update();
	void Render();

	//ComPtr<ID3D11Buffer> m_constantBuffer;

private:
	void CreateMesh();

	std::vector<FogVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	//SkyboxConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};