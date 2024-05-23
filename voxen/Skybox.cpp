#include "Skybox.h"
#include "Graphics.h"
#include "DXUtils.h"

Skybox::Skybox()
	: m_speed(0.0f), m_stride(sizeof(SkyboxVertex)), m_offset(0), m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}

Skybox::~Skybox() {}

bool Skybox::Initialize(float scale, float speed)
{
	// make block
	m_speed = speed;

	CreateBox(scale);

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
	//m_constantData.sunDir = Vector3::Transform(m_constantData.sunDir, Matrix::CreateRotationZ(0.3f * 3.141592f));
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in skybox" << std::endl;
		return false;
	}

	return true;
}

void Skybox::Update(float dt) 
{
	m_constantData.sunDir = Vector3::Transform(m_constantData.sunDir, Matrix::CreateRotationZ(dt * m_speed));
	m_constantData.sunDir.Normalize();
	DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);
}

void Skybox::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	Graphics::context->PSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());

	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::sunSRV.Get(),
		Graphics::moonSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}  

void Skybox::CreateBox(float scale)
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