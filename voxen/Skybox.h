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
	void Update(float dt);
	void Render();

	ComPtr<ID3D11Buffer> m_constantBuffer;
	Vector3 GetSun() { return m_constantData.sunDir; };
	uint32_t GetTime() { return m_constantData.dateTime; };

private:
	void CreateMesh(float scale);

	uint32_t m_dateTime;
	float m_speed;

	const uint32_t DATE_CYCLE_AMOUNT = 24000;
	const uint32_t DATE_REAL_TIME = 30; // 60
	const float DATE_TIME_SPEED = (float)DATE_CYCLE_AMOUNT / DATE_REAL_TIME;
	
	// normal color
	const Vector3 NORMAL_DAY_HORIZON = Vector3(0.67f, 0.82f, 1.0f);
	const Vector3 NORMAL_DAY_ZENITH = Vector3(0.52f, 0.67f, 1.0f);

	const Vector3 NORMAL_NIGHT_HORIZON = Vector3(0.04f, 0.05f, 0.09f);
	const Vector3 NORMAL_NIGHT_ZENITH = Vector3(0.0f, 0.0f, 0.01f);
	
	// sun color
	const Vector3 SUN_DAY_HORIZON = Vector3(0.60f, 0.74f, 1.0f);
	const Vector3 SUN_DAY_ZENITH = Vector3(0.32f, 0.45f, 1.0f);

	const Vector3 SUN_SUNRISE_HORIZON = Vector3(0.72f, 0.60f, 0.34f);
	const Vector3 SUN_SUNRISE_ZENITH = Utils::Lerp(SUN_DAY_ZENITH, NORMAL_NIGHT_ZENITH, 15.0f / 27.0f);

	const Vector3 SUN_SUNSET_HORIZON = Vector3(0.64f, 0.26f, 0.04f);
	const Vector3 SUN_SUNSET_ZENITH = Utils::Lerp(SUN_DAY_ZENITH, NORMAL_NIGHT_ZENITH, 15.0f / 27.0f);

	std::vector<SkyboxVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	SkyboxConstantData m_constantData;

	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};