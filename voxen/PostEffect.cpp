#include "PostEffect.h"
#include "Graphics.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

PostEffect::PostEffect()
	: m_stride(sizeof(SamplingVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr){};

PostEffect::~PostEffect(){};

bool PostEffect::Initialize()
{
	MeshGenerator::CreateSampleSquareMesh(m_vertices, m_indices);

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create fog vertex buffer" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create fog index buffer" << std::endl;
		return false;
	}

	m_postEffectConstantData.dx = 1.0f / (float)App::MIRROR_WIDTH;
	m_postEffectConstantData.dy = 1.0f / (float)App::MIRROR_HEIGHT;
	if (!DXUtils::CreateConstantBuffer(m_postEffectConstantBuffer, m_postEffectConstantData)) {
		std::cout << "failed create blur constant buffer" << std::endl;
		return false;
	}

	return true;
}

void PostEffect::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}

void PostEffect::RenderFog()
{
	Graphics::context->ResolveSubresource(Graphics::basicResolvedBuffer.Get(), 0,
		Graphics::basicRenderBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::basicResolvedSRV.Get(),
		Graphics::depthOnlySRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::context->OMSetRenderTargets(1, Graphics::basicRTV.GetAddressOf(), nullptr);

	Render();
}

void PostEffect::BlurMirror(int loopCount)
{
	Graphics::context->PSSetConstantBuffers(2, 1, m_postEffectConstantBuffer.GetAddressOf());

	for (int i = 0; i < loopCount; ++i) {
		Graphics::context->OMSetRenderTargets(
			1, Graphics::mirrorWorldBlurRTV[0].GetAddressOf(), nullptr);
		if (i != 0)
			Graphics::context->PSSetShaderResources(
				0, 1, Graphics::mirrorWorldBlurSRV[1].GetAddressOf());
		else
			Graphics::context->PSSetShaderResources(0, 1, Graphics::mirrorWorldSRV.GetAddressOf());
		Graphics::context->PSSetShader(Graphics::blurXPS.Get(), nullptr, 0);
		
		Render();

		if (i != loopCount - 1)
			Graphics::context->OMSetRenderTargets(
				1, Graphics::mirrorWorldBlurRTV[1].GetAddressOf(), nullptr);
		else
			Graphics::context->OMSetRenderTargets(
				1, Graphics::mirrorWorldRTV.GetAddressOf(), nullptr);
		Graphics::context->PSSetShaderResources(
			0, 1, Graphics::mirrorWorldBlurSRV[0].GetAddressOf());
		Graphics::context->PSSetShader(Graphics::blurYPS.Get(), nullptr, 0);
		Render();
	}
}