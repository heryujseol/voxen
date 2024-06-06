#include "Fog.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <algorithm>

Fog::Fog()
	: m_stride(sizeof(FogVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr)
{};

Fog::~Fog(){};

bool Fog::Initialize() {
	CreateMesh();

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create fog vertex buffer in cloud" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create fog index buffer in cloud" << std::endl;
		return false;
	}

	return true;
}

void Fog::Render()
{
	Graphics::context->ResolveSubresource(Graphics::postEffectResolvedBuffer.Get(), 0,
		Graphics::basicRenderBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
	//Graphics::context->OMSetRenderTargets(1, Graphics::basicRTV.GetAddressOf(), nullptr);

	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::postEffectSRV.Get(),
		Graphics::depthOnlySRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}

void Fog::CreateMesh()
{
	FogVertex v;

	v.position = Vector3(-1.0f, 1.0f, 0.0f);
	v.texcoord = Vector2(0.0f, 0.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, 1.0f, 0.0f);
	v.texcoord = Vector2(1.0f, 0.0f);
	m_vertices.push_back(v);
	v.position = Vector3(1.0f, -1.0f, 0.0f);
	v.texcoord = Vector2(1.0f, 1.0f);
	m_vertices.push_back(v);
	v.position = Vector3(-1.0f, -1.0f, 0.0f);
	v.texcoord = Vector2(0.0f, 1.0f);
	m_vertices.push_back(v);

	m_indices.push_back(0);
	m_indices.push_back(1);
	m_indices.push_back(2);

	m_indices.push_back(0);
	m_indices.push_back(2);
	m_indices.push_back(3);
}