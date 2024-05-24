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

	bool Initialize(float scale, float speed);
	void Update(float dt, Vector3 eyeDir);
	void Render();

	ComPtr<ID3D11Buffer> m_constantBuffer;

private:
	void CreateMesh(float scale);

	float m_speed;

	const float SHOW_BOUNDARY = -0.2f;
	const float SECTION_BOUNDARY = 0.1f;
	const Vector3 HORIZON_DAY = Vector3(0.68f, 0.83f, 1.0f);
	const Vector3 HORIZON_NIGHT = Vector3(0.05f, 0.05f, 0.2f);
	const Vector3 HORIZON_SUNRISE = Vector3(0.8f, 0.4f, 0.2f);
	const Vector3 HORIZON_SUNSET = Vector3(0.9f, 0.6f, 0.2f);
	const Vector3 ZENITH_DAY = Vector3(0.51f, 0.69f, 1.0f);
	const Vector3 ZENITH_NIGHT = Vector3(0.0f, 0.0f, 0.1f);

	std::vector<SkyboxVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	SkyboxConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};