#include "Skybox.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <algorithm>

Skybox::Skybox()
	: m_speed(0.0f), m_stride(sizeof(SkyboxVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr)
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
	m_constantData.showAltitudeBoundary = SHOW_BOUNDARY;
	m_constantData.sectionAltitudeBounary = SECTION_BOUNDARY;
	m_constantData.sunDir = Vector3(1.0f, 0.0f, 0.0f);
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in skybox" << std::endl;
		return false;
	}

	return true;
}

void Skybox::Update(float dt, Vector3 eyeDir)
{
	// sunDir
	Vector3 sunDir =
		Vector3::Transform(m_constantData.sunDir, Matrix::CreateRotationZ(dt * m_speed));
	sunDir.Normalize();

	float sunAltitude = std::clamp(
		(std::acos(sunDir.y) - (Utils::PI * 0.5f)) * (-2.0f * Utils::invPI), -1.0f, 1.0f);

	// 태양 위치에 따른 빠른 색 변환 (고도가 0에서 증가할 때 빠르게 밤낮이 바뀌기 위함)
	float exp = ((sunAltitude >= 0 ? 1.0f : -1.0f) * powf(abs(sunAltitude), 0.6f) + 1.0f) * 0.5f;
	Vector3 zenithColor = Utils::Lerp(ZENITH_NIGHT, ZENITH_DAY, exp);
	Vector3 normalHorizonColor = Utils::Lerp(HORIZON_NIGHT, HORIZON_DAY, exp);

	// 태양의 고도가 낮을 때만 sun컬러를 결정하도록 선택
	// zenith와 horizon 구별 고도 고려
	Vector3 sunHorizonX = Utils::Lerp(HORIZON_SUNSET, HORIZON_SUNRISE, (sunDir.x + 1.0f) * 0.5f);
	Vector3 sunHorizon = Utils::Lerp(
		sunHorizonX, normalHorizonColor, powf(abs(sunAltitude - SECTION_BOUNDARY), 0.3f));

	// 바라보는 방향에 대한 비등방성
	float sunDirWeight =
		sunAltitude > SHOW_BOUNDARY ? Utils::HenyeyGreensteinPhase(sunDir, eyeDir, 0.625f) : 0.0f;
	Vector3 horizonColor = Utils::Lerp(normalHorizonColor, sunHorizon, sunDirWeight);

	float sunClampedAlt = std::clamp(sunAltitude, SHOW_BOUNDARY, SECTION_BOUNDARY);
	float sunStrengthWeight = (sunClampedAlt - SHOW_BOUNDARY) / (SECTION_BOUNDARY - SHOW_BOUNDARY);

	float moonClampedAlt = std::clamp(-sunAltitude, SHOW_BOUNDARY, SECTION_BOUNDARY);
	float moonStrengthWeight =
		(moonClampedAlt - SHOW_BOUNDARY) / (SECTION_BOUNDARY - SHOW_BOUNDARY);

	m_constantData.sunDir = sunDir;
	m_constantData.sunAltitude = sunAltitude;
	m_constantData.sunStrength = Vector3(Utils::Smootherstep(0.0f, 1.0f, sunStrengthWeight));
	m_constantData.moonStrength = Vector3(Utils::Smootherstep(0.0f, 1.0f, moonStrengthWeight));
	m_constantData.horizonColor = horizonColor;
	m_constantData.zenithColor = zenithColor;
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
	// 왼쪽
	v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);


	// 오른쪽
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);


	// 아랫면
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f) * scale;
	m_vertices.push_back(v);

	// 윗면
	v.position = Vector3(-1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);

	// 앞면
	v.position = Vector3(-1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f) * scale;
	m_vertices.push_back(v);

	// 뒷면
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