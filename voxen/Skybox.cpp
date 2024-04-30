#include "Skybox.h"
#include "Graphics.h"
#include "DXUtils.h"

Skybox::Skybox()
	: m_stride(sizeof(SkyboxVertex)), m_offset(0), m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}

Skybox::~Skybox() {}

bool Skybox::Initialize(float scale)
{
	// make block
	CreateBox();

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create vertex buffer in skybox" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create index buffer in skybox" << std::endl;
		return false;
	}

	m_constantData.scale = scale;
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in skybox" << std::endl;
		return false;
	}

	return true;
}

void Skybox::Update(float dt) 
{

}

void Skybox::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}

void Skybox::CreateBox()
{
	SkyboxVertex v;
	// ¿ÞÂÊ
	v.face = 0;
	v.position = Vector3(-1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f);
	m_vertices.push_back(v);


	// ¿À¸¥ÂÊ
	v.face = 1;
	v.position = Vector3(1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);


	// ¾Æ·§¸é
	v.face = 2;
	v.position = Vector3(-1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f);
	m_vertices.push_back(v);

	// À­¸é
	v.face = 3;
	v.position = Vector3(-1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);

	// ¾Õ¸é
	v.face = 4;
	v.position = Vector3(-1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, -1.0f);
	m_vertices.push_back(v);

	// µÞ¸é
	v.face = 5;
	v.position = Vector3(1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, 1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 1.0f);
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