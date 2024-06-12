#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "GraphicsPSO.h"

using namespace Microsoft::WRL;

namespace Graphics {
	// Graphics Core
	extern ComPtr<ID3D11Device> device;
	extern ComPtr<ID3D11DeviceContext> context;
	extern ComPtr<IDXGISwapChain> swapChain;


	// Input Layout
	extern ComPtr<ID3D11InputLayout> basicIL;
	extern ComPtr<ID3D11InputLayout> skyboxIL;
	extern ComPtr<ID3D11InputLayout> cloudIL;
	extern ComPtr<ID3D11InputLayout> samplingIL;
	extern ComPtr<ID3D11InputLayout> instanceIL;
	extern ComPtr<ID3D11InputLayout> depthOnlyIL;


	// Vertex Shader
	extern ComPtr<ID3D11VertexShader> basicVS;
	extern ComPtr<ID3D11VertexShader> skyboxVS;
	extern ComPtr<ID3D11VertexShader> cloudVS;
	extern ComPtr<ID3D11VertexShader> samplingVS;
	extern ComPtr<ID3D11VertexShader> depthOnlyVS;
	extern ComPtr<ID3D11VertexShader> instanceVS;


	// Pixel Shader
	extern ComPtr<ID3D11PixelShader> basicPS;
	extern ComPtr<ID3D11PixelShader> skyboxPS;
	extern ComPtr<ID3D11PixelShader> cloudPS;
	extern ComPtr<ID3D11PixelShader> samplingPS;
	extern ComPtr<ID3D11PixelShader> instancePS;
	extern ComPtr<ID3D11PixelShader> depthOnlyPS;
	extern ComPtr<ID3D11PixelShader> alphaClipPS;


	// Rasterizer State
	extern ComPtr<ID3D11RasterizerState> solidRS;
	extern ComPtr<ID3D11RasterizerState> wireRS;
	extern ComPtr<ID3D11RasterizerState> noneCullRS;
	extern ComPtr<ID3D11RasterizerState> postEffectRS;

	// Sampler State
	extern ComPtr<ID3D11SamplerState> pointClampSS;
	extern ComPtr<ID3D11SamplerState> linearWrapSS;
	extern ComPtr<ID3D11SamplerState> linearClampSS;


	// Depth Stencil State
	extern ComPtr<ID3D11DepthStencilState> basicDSS;
	extern ComPtr<ID3D11DepthStencilState> postEffectDSS;

	
	// Blend State
	extern ComPtr<ID3D11BlendState> alphaBS;


	// RTV & Buffer
	extern ComPtr<ID3D11Texture2D> backBuffer;
	extern ComPtr<ID3D11RenderTargetView> backBufferRTV;

	extern ComPtr<ID3D11Texture2D> basicRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> basicRTV;

	extern ComPtr<ID3D11Texture2D> cloudRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> cloudRTV;

	extern ComPtr<ID3D11Texture2D> postEffectBuffer;
	extern ComPtr<ID3D11RenderTargetView> postEffectRTV;
	

	// DSV & Buffer
	extern ComPtr<ID3D11Texture2D> basicDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> basicDSV;

	extern ComPtr<ID3D11Texture2D> depthOnlyBuffer;
	extern ComPtr<ID3D11DepthStencilView> depthOnlyDSV;


	// SRV & Buffer
	extern ComPtr<ID3D11Texture2D> atlasMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	extern ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	extern ComPtr<ID3D11Texture2D> sunBuffer;
	extern ComPtr<ID3D11ShaderResourceView> sunSRV;
	extern ComPtr<ID3D11Texture2D> moonBuffer;
	extern ComPtr<ID3D11ShaderResourceView> moonSRV;

	extern ComPtr<ID3D11Texture2D> cloudResolvedBuffer;
	extern ComPtr<ID3D11ShaderResourceView> cloudSRV;

	extern ComPtr<ID3D11ShaderResourceView> depthOnlySRV;

	extern ComPtr<ID3D11Texture2D> postEffectResolvedBuffer;
	extern ComPtr<ID3D11ShaderResourceView> postEffectSRV;


	// Viewport
	extern D3D11_VIEWPORT basicViewport;


	// device, context, swapChain
	extern bool InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd, UINT width, UINT height);
	

	// RTV, DSV, SRV (+ UAV ...)
	extern bool InitGraphicsBuffer(UINT width, UINT height);
	extern bool InitRenderTargetBuffers(UINT width, UINT height);
	extern bool InitDepthStencilBuffers(UINT width, UINT height);
	extern bool InitShaderResourceBuffers(UINT width, UINT height);
	

	// VS, IL, PS, RS, SS, DSS (+ HS, DS, GS, BS ...)
	extern bool InitGraphicsState();
	extern bool InitVertexShaderAndInputLayouts();
	extern bool InitGeometryShaders();
	extern bool InitPixelShaders();
	extern bool InitRasterizerStates();
	extern bool InitSamplerStates();
	extern bool InitDepthStencilStates();
	extern bool InitBlendStates();


	// PSO
	extern void InitGraphicsPSO();
	extern void SetPipelineStates(GraphicsPSO& pso);
	extern GraphicsPSO basicPSO;
	extern GraphicsPSO basicWirePSO;
	extern GraphicsPSO skyboxPSO;
	extern GraphicsPSO cloudPSO;
	extern GraphicsPSO cloudBlendPSO;
	extern GraphicsPSO depthOnlyPSO;
	extern GraphicsPSO postEffectPSO;
	extern GraphicsPSO instancePSO;
	extern GraphicsPSO semiAlphaPSO;
	extern GraphicsPSO transparencyPSO;
}
