#pragma once

#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Graphics {
	// Graphics Core
	extern ComPtr<ID3D11Device> device;
	extern ComPtr<ID3D11DeviceContext> context;
	extern ComPtr<IDXGISwapChain> swapChain;

	// Input Layout
	extern ComPtr<ID3D11InputLayout> basicIL;


	// Vertex Shader
	extern ComPtr<ID3D11VertexShader> basicVS;
	extern ComPtr<ID3D11VertexShader> skyboxVS;


	// Pixel Shader
	extern ComPtr<ID3D11PixelShader> basicPS;
	extern ComPtr<ID3D11PixelShader> skyboxPS;


	// Rasterizer State
	extern ComPtr<ID3D11RasterizerState> solidRS;
	extern ComPtr<ID3D11RasterizerState> wireRS;


	// Sampler State
	extern ComPtr<ID3D11SamplerState> pointClampSS;
	extern ComPtr<ID3D11SamplerState> linearWrapSS;


	// Depth Stencil State
	extern ComPtr<ID3D11DepthStencilState> basicDSS;


	// RTV & Buffer
	extern ComPtr<ID3D11Texture2D> backBuffer;
	extern ComPtr<ID3D11RenderTargetView> backBufferRTV;

	extern ComPtr<ID3D11Texture2D> basicRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> basicRTV;

	
	// DSV & Buffer
	extern ComPtr<ID3D11Texture2D> basicDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> basicDSV;


	// SRV & Buffer
	extern ComPtr<ID3D11Texture2D> atlasMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	extern ComPtr<ID3D11Texture2D> biomeColorMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> biomeColorMapSRV;

	extern ComPtr<ID3D11Texture2D> topBuffer;
	extern ComPtr<ID3D11ShaderResourceView> topSRV;

	extern ComPtr<ID3D11Texture2D> sideBuffer;
	extern ComPtr<ID3D11ShaderResourceView> sideSRV;

	extern ComPtr<ID3D11Texture2D> dirtBuffer;
	extern ComPtr<ID3D11ShaderResourceView> dirtSRV;


	// Viewport
	extern D3D11_VIEWPORT basicViewport;


	// device, context, swapChain
	extern bool InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd, UINT width, UINT height);
	

	// RTV(+viewport), DSV, SRV (+ UAV ...)
	extern bool InitGraphicsBuffer(UINT width, UINT height);
	extern bool InitRenderTargetBuffers(UINT width, UINT height);
	extern bool InitDepthStencilBuffers(UINT width, UINT height);
	extern bool InitShaderResourceBuffers();
	

	// VS, IL, PS, RS, SS, DSS (+ HS, DS, GS, BS ...)
	extern bool InitGraphicsState();
	extern bool InitVertexShaderAndInputLayouts();
	extern bool InitPixelShaders();
	extern bool InitRasterizerStates();
	extern bool InitSamplerStates();
	extern bool InitDepthStencilStates();
}
