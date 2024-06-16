#pragma once

#include "Structure.h"
#include "Camera.h"

#include <d3d11.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class Cloud {
public:
	Cloud();
	~Cloud();

	bool Initialize(Vector3 cameraPosition);
	void Update(float dt, Vector3 cameraPosition);
	void Render();
	void Blend();

private:
	
	bool BuildCloud();
	bool BuildSquare();
	
	static const int CLOUD_DATA_MAP_SIZE = 512;
	static const int CLOUD_SCALE_SIZE = 16;
	static const int CLOUD_MAP_SIZE = 64;

	std::vector<std::vector<bool>> m_map;
	std::vector<std::vector<bool>> m_dataMap;
	Vector3 m_mapCenterPosition;
	Vector3 m_mapDataOffset;
	float m_speed;
	float m_height;

	std::vector<CloudVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	CloudConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;


	// blend
	std::vector<SamplingVertex> m_samplingVertices;
	std::vector<uint32_t> m_samplingIndices;

	UINT m_samplingStride;
	UINT m_samplingOffset;
	ComPtr<ID3D11Buffer> m_samplingVertexBuffer;
	ComPtr<ID3D11Buffer> m_samplingIndexBuffer;
};