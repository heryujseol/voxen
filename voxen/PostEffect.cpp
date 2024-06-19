#include "PostEffect.h"
#include "Graphics.h"
#include "DXUtils.h"
#include "MeshGenerator.h"

#include <algorithm>

PostEffect::PostEffect()
	: m_stride(sizeof(SamplingVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr)
{};

PostEffect::~PostEffect(){};

bool PostEffect::Initialize()
{
	MeshGenerator::CreateSampleSquareMesh(m_vertices, m_indices);

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

void PostEffect::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);
	Graphics::context->OMSetRenderTargets(1, Graphics::basicRTV.GetAddressOf(), nullptr);

	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::postEffectSRV.Get(),
		Graphics::depthOnlySRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}