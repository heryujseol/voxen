#include "Graphics.h"
#include "DXUtils.h"

#include <iostream>

namespace Graphics {
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;

	// Input Layout
	ComPtr<ID3D11InputLayout> basicIL;


	// Vertex Shader
	ComPtr<ID3D11VertexShader> basicVS;
	ComPtr<ID3D11VertexShader> skyboxVS;


	// Pixel Shader
	ComPtr<ID3D11PixelShader> basicPS;
	ComPtr<ID3D11PixelShader> skyboxPS;


	// Rasterizer State
	ComPtr<ID3D11RasterizerState> solidRS;
	ComPtr<ID3D11RasterizerState> wireRS;


	// Sampler State
	ComPtr<ID3D11SamplerState> pointClampSS;
	ComPtr<ID3D11SamplerState> linearWrapSS;


	// Depth Stencil State
	ComPtr<ID3D11DepthStencilState> basicDSS;


	// RTV & Buffer
	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	ComPtr<ID3D11Texture2D> basicRenderBuffer;
	ComPtr<ID3D11RenderTargetView> basicRTV;


	// DSV & Buffer
	ComPtr<ID3D11Texture2D> basicDepthBuffer;
	ComPtr<ID3D11DepthStencilView> basicDSV;


	// SRV & Buffer
	ComPtr<ID3D11Texture2D> atlasMapBuffer;
	ComPtr<ID3D11ShaderResourceView> atlasMapSRV;
	ComPtr<ID3D11Texture2D> biomeColorMapBuffer;
	ComPtr<ID3D11ShaderResourceView> biomeColorMapSRV;
	ComPtr<ID3D11Texture2D> topBuffer;
	ComPtr<ID3D11ShaderResourceView> topSRV;
	ComPtr<ID3D11Texture2D> sideBuffer;
	ComPtr<ID3D11ShaderResourceView> sideSRV;
	ComPtr<ID3D11Texture2D> dirtBuffer;
	ComPtr<ID3D11ShaderResourceView> dirtSRV;


	// Viewport
	D3D11_VIEWPORT basicViewport;


	// PSO
	void InitGraphicsPSO();
	void SetPipelineStates(GraphicsPSO& pso);

	GraphicsPSO basicPSO;
	GraphicsPSO basicWirePSO;
	GraphicsPSO skyboxPSO;
}


// Function
bool Graphics::InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd, UINT width, UINT height)
{
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

	UINT deviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT ret = D3D11CreateDevice(0, driverType, 0, deviceFlags, levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create device and context" << std::endl;
		return false;
	}

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = pixelFormat;
	desc.SampleDesc.Count = 1; // backbuffer는 멀티 샘플링 하지 않음
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.OutputWindow = hwnd;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ret = D3D11CreateDeviceAndSwapChain(NULL, driverType, 0, deviceFlags, levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION, &desc, swapChain.GetAddressOf(), device.GetAddressOf(), &featureLevel,
		context.GetAddressOf());

	if (FAILED(ret)) {
		std::cout << "failed create swapchain" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGraphicsBuffer(UINT width, UINT height)
{
	if (!InitRenderTargetBuffers(width, height))
		return false;

	if (!InitDepthStencilBuffers(width, height))
		return false;

	if (!InitShaderResourceBuffers())
		return false;

	return true;
}

bool Graphics::InitRenderTargetBuffers(UINT width, UINT height)
{
	// backBuffer
	swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	HRESULT ret =
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, backBufferRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create render target view" << std::endl;
		return false;
	}

	// Basic RTV
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_BIND_FLAG bindFlag = D3D11_BIND_RENDER_TARGET;
	if (!DXUtils::CreateTextureBuffer(basicRenderBuffer, width, height, true, format, bindFlag)) {
		std::cout << "failed create render target buffer" << std::endl;
		return false;
	}
	ret = Graphics::device->CreateRenderTargetView(
		basicRenderBuffer.Get(), nullptr, basicRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create render target view" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitDepthStencilBuffers(UINT width, UINT height)
{
	// basic DSV
	DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D11_BIND_FLAG bindFlag = D3D11_BIND_DEPTH_STENCIL;
	if (!DXUtils::CreateTextureBuffer(basicDepthBuffer, width, height, true, format, bindFlag)) {
		std::cout << "failed create depth stencil buffer" << std::endl;
		return false;
	}
	HRESULT ret = Graphics::device->CreateDepthStencilView(
		basicDepthBuffer.Get(), nullptr, basicDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create depth stencil view" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitShaderResourceBuffers()
{
	if (!DXUtils::CreateTextureFromFile(
			atlasMapBuffer, atlasMapSRV, "../assets/grass_block_top.png")) {
		std::cout << "failed create texture from file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTextureFromFile(
			biomeColorMapBuffer, biomeColorMapSRV, "../assets/grass_color_map.png")) {
		std::cout << "failed create texture from file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTextureFromFile(topBuffer, topSRV, "../assets/grass_block_top.png")) {
		std::cout << "failed create texture from file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTextureFromFile(
			sideBuffer, sideSRV, "../assets/grass_block_side_overlay.png")) {
		std::cout << "failed create texture from file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTextureFromFile(dirtBuffer, dirtSRV, "../assets/dirt.png")) {
		std::cout << "failed create texture from file" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGraphicsState()
{
	if (!InitVertexShaderAndInputLayouts())
		return false;

	if (!InitPixelShaders())
		return false;

	if (!InitRasterizerStates())
		return false;

	if (!InitSamplerStates())
		return false;

	if (!InitDepthStencilStates())
		return false;

	return true;
}

bool Graphics::InitVertexShaderAndInputLayouts()
{
	// BasicVS & BasicIL
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"BasicVS.hlsl", basicVS, basicIL, elementDesc)) {
		std::cout << "failed create vs" << std::endl;
		return false;
	}

	// SkyBox
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"SkyboxVS.hlsl", skyboxVS, basicIL, elementDesc)) {
		std::cout << "failed create vs" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitPixelShaders()
{
	// BasicPS
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicPS)) {
		std::cout << "failed create ps" << std::endl;
		return false;
	}

	// SkyboxPS
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxPS)) {
		std::cout << "failed create ps" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitRasterizerStates()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;
	rastDesc.MultisampleEnable = true;

	// solidRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	HRESULT ret = Graphics::device->CreateRasterizerState(&rastDesc, solidRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create solid RS" << std::endl;
		return false;
	}

	// wireRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, wireRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create wire RS" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitSamplerStates()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	// point clamp
	desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	HRESULT ret = Graphics::device->CreateSamplerState(&desc, pointClampSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create point clamp SS" << std::endl;
		return false;
	}

	// linear wrap
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ret = Graphics::device->CreateSamplerState(&desc, linearWrapSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create linear Wrap SS" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;

	// basic DSS
	HRESULT ret = Graphics::device->CreateDepthStencilState(&desc, basicDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic DSS";
		return false;
	}

	return true;
}

void Graphics::InitGraphicsPSO()
{
	// basicPSO
	basicPSO.inputLayout = basicIL;
	basicPSO.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	basicPSO.vertexShader = basicVS;
	basicPSO.rasterizerState = solidRS;
	basicPSO.pixelShader = basicPS;
	basicPSO.samplerStates.push_back(pointClampSS.Get());
	basicPSO.depthStencilState = basicDSS;

	// basic wire PSO
	basicWirePSO = basicPSO;
	basicWirePSO.rasterizerState = wireRS;

	// skyboxPSO
	skyboxPSO = basicPSO;
	skyboxPSO.vertexShader = skyboxVS;
	skyboxPSO.pixelShader = skyboxPS;
	skyboxPSO.samplerStates.clear();
}

void Graphics::SetPipelineStates(GraphicsPSO& pso)
{
	context->IASetInputLayout(pso.inputLayout.Get());
	context->IASetPrimitiveTopology(pso.topology);

	context->VSSetShader(pso.vertexShader.Get(), nullptr, 0);

	context->RSSetState(pso.rasterizerState.Get());

	context->PSSetShader(pso.pixelShader.Get(), nullptr, 0);

	if (pso.samplerStates.empty())
		context->PSSetSamplers(0, 0, nullptr);
	else
		context->PSSetSamplers(0, (UINT)pso.samplerStates.size(), pso.samplerStates.data());

	context->OMSetDepthStencilState(pso.depthStencilState.Get(), 0);
}