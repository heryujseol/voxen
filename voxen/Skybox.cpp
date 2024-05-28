#include "Skybox.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <algorithm>

Skybox::Skybox()
	: m_dateTime(0), m_speed(0.0f), m_stride(sizeof(SkyboxVertex)), m_offset(0),
	  m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}

Skybox::~Skybox() {}

bool Skybox::Initialize(float scale, float speed)
{
	m_speed = speed;

	CreateMesh(scale);

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create vertex buffer in skybox" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create index buffer in skybox" << std::endl;
		return false;
	}

	m_constantData.skyScale = scale;
	m_constantData.sunDir = Vector3(1.0f, 0.0f, 0.0f);
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in skybox" << std::endl;
		return false;
	}

	return true;
}

void Skybox::Update(float dt)
{
	static float acc = 0.0f;

	// dateTime
	acc += DATE_TIME_SPEED * dt;
	m_dateTime = (uint32_t)acc;
	m_dateTime %= DATE_CYCLE_AMOUNT;

	// sunDir
	float angle = (float)m_dateTime / DATE_CYCLE_AMOUNT * 2.0f * Utils::PI;
	Vector3 sunDir = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	sunDir.Normalize();

	// set color & strength
	Vector3 normalHorizonColor = Vector3(0.0f);
	Vector3 normalZenithColor = Vector3(0.0f);
	Vector3 sunHorizonColor = Vector3(0.0f);
	Vector3 sunZenithColor = Vector3(0.0f);
	float sunStrength = 0.0f;

	if (1000 <= m_dateTime && m_dateTime < 11000) { // day
		normalHorizonColor = NORMAL_DAY_HORIZON;
		normalZenithColor = NORMAL_DAY_ZENITH;
		sunHorizonColor = SUN_DAY_HORIZON;
		sunZenithColor = SUN_DAY_ZENITH;

		sunStrength = 1.0f;
	}
	else if (13700 <= m_dateTime && m_dateTime < 22300) { // night
		normalHorizonColor = NORMAL_NIGHT_HORIZON;
		normalZenithColor = NORMAL_NIGHT_ZENITH;
		sunHorizonColor = NORMAL_NIGHT_HORIZON;
		sunZenithColor = NORMAL_NIGHT_ZENITH;

		sunStrength = 0.0f;
	}
	else { // mix
		if (m_dateTime < 1000)
			m_dateTime += DATE_CYCLE_AMOUNT;

		// normal color
		if (11000 <= m_dateTime && m_dateTime < 13700) {
			float w = (float)(m_dateTime - 11000) / 2700.0f;
			normalHorizonColor = Utils::Lerp(NORMAL_DAY_HORIZON, NORMAL_NIGHT_HORIZON, w);
			normalZenithColor = Utils::Lerp(NORMAL_DAY_ZENITH, NORMAL_NIGHT_ZENITH, w);

			sunStrength = Utils::Smootherstep(0.0f, 1.0f, 1.0f - w);
		} // 11000 ~ 13700 | 22300 ~ 25000
		else if (22300 <= m_dateTime && m_dateTime <= 25000) {
			float w = (float)(m_dateTime - 22300) / 2700.0f;
			normalHorizonColor = Utils::Lerp(NORMAL_NIGHT_HORIZON, NORMAL_DAY_HORIZON, w);
			normalZenithColor = Utils::Lerp(NORMAL_NIGHT_ZENITH, NORMAL_DAY_ZENITH, w);

			sunStrength = Utils::Smootherstep(0.0f, 1.0f, w);
		}

		// sun color
		if (11000 <= m_dateTime && m_dateTime < 12500) { // day ~ sunset
			float w = (float)(m_dateTime - 11000) / 1500.0f;
			sunHorizonColor = Utils::Lerp(SUN_DAY_HORIZON, SUN_SUNSET_HORIZON, w);
			sunZenithColor = Utils::Lerp(SUN_DAY_ZENITH, SUN_SUNSET_ZENITH, w);
		}
		else if (12500 <= m_dateTime && m_dateTime < 13700) { // sunset ~ night
			float w = (float)(m_dateTime - 12500) / 1200.0f;
			sunHorizonColor = Utils::Lerp(SUN_SUNSET_HORIZON, NORMAL_NIGHT_HORIZON, w);
			sunZenithColor = Utils::Lerp(SUN_SUNSET_ZENITH, NORMAL_NIGHT_ZENITH, w);
		}
		else if (22300 <= m_dateTime && m_dateTime < 23500) { // night ~ sunrise
			float w = (float)(m_dateTime - 22300) / 1200.0f;
			sunHorizonColor = Utils::Lerp(NORMAL_NIGHT_HORIZON, SUN_SUNRISE_HORIZON, w);
			sunZenithColor = Utils::Lerp(NORMAL_NIGHT_ZENITH, SUN_SUNRISE_ZENITH, w);
		}
		else { // sunrise ~ day
			float w = (float)(m_dateTime - 23500) / 1500.0f;
			sunHorizonColor = Utils::Lerp(SUN_SUNRISE_HORIZON, SUN_DAY_HORIZON, w);
			sunZenithColor = Utils::Lerp(SUN_SUNRISE_ZENITH, SUN_DAY_ZENITH, w);
		}
	}

	m_constantData.dateTime = m_dateTime % DATE_CYCLE_AMOUNT;
	m_constantData.sunDir = sunDir;
	m_constantData.normalHorizonColor = normalHorizonColor;
	m_constantData.normalZenithColor = normalZenithColor;
	m_constantData.sunHorizonColor = sunHorizonColor;
	m_constantData.sunZenithColor = sunZenithColor;
	m_constantData.sunStrength = sunStrength;
	m_constantData.moonStrength = 1.0f - sunStrength;

	DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);
}

void Skybox::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::sunSRV.Get(),
		Graphics::moonSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}

void Skybox::CreateMesh(float scale)
{
	SkyboxVertex v;
	// ¿ÞÂÊ
	v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);


	// ¿À¸¥ÂÊ
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);


	// ¾Æ·§¸é
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);

	// À­¸é
	v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);

	// ¾Õ¸é
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);

	// µÞ¸é
	v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);

	for (int i = 0; i < 24; i += 4) {
		m_indices.push_back(i);
		m_indices.push_back(i + 1);
		m_indices.push_back(i + 2);

		m_indices.push_back(i);
		m_indices.push_back(i + 2);
		m_indices.push_back(i + 3);
	}

	std::reverse(m_indices.begin(), m_indices.end()); // for front CW at inner
}