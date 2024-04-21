#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class GraphicsPSO
{
public:
	GraphicsPSO();
	GraphicsPSO(const GraphicsPSO& rhs);
	GraphicsPSO operator=(const GraphicsPSO& other);
	~GraphicsPSO();
	
	ComPtr<ID3D11InputLayout> inputLayout;
	D3D11_PRIMITIVE_TOPOLOGY topology;

	ComPtr<ID3D11VertexShader> vertexShader;

	ComPtr<ID3D11RasterizerState> rasterizerState;

	ComPtr<ID3D11PixelShader> pixelShader;
	
	std::vector<ID3D11SamplerState *> samplerStates;

	ComPtr<ID3D11DepthStencilState> depthStencilState;
	

	/*
	ComPtr<ID3D11HullShader> m_hullShader;
    ComPtr<ID3D11DomainShader> m_domainShader;
    ComPtr<ID3D11GeometryShader> m_geometryShader;
	ComPtr<ID3D11BlendState> m_blendState;
	float m_blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	UINT m_stencilRef = 0;
	*/
};