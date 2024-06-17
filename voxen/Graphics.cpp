#include "Graphics.h"
#include "DXUtils.h"

#include <iostream>

namespace Graphics {
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;

	// Input Layout
	ComPtr<ID3D11InputLayout> basicIL;
	ComPtr<ID3D11InputLayout> skyboxIL;
	ComPtr<ID3D11InputLayout> cloudIL;
	ComPtr<ID3D11InputLayout> samplingIL;
	ComPtr<ID3D11InputLayout> instanceIL;


	// Vertex Shader
	ComPtr<ID3D11VertexShader> basicVS;
	ComPtr<ID3D11VertexShader> skyboxVS;
	ComPtr<ID3D11VertexShader> skyboxEnvMapVS;
	ComPtr<ID3D11VertexShader> cloudVS;
	ComPtr<ID3D11VertexShader> samplingVS;
	ComPtr<ID3D11VertexShader> instanceVS;


	// Geometry Shader
	ComPtr<ID3D11GeometryShader> skyboxEnvMapGS;


	// Pixel Shader
	ComPtr<ID3D11PixelShader> basicPS;
	ComPtr<ID3D11PixelShader> skyboxPS;
	ComPtr<ID3D11PixelShader> skyboxEnvMapPS;
	ComPtr<ID3D11PixelShader> cloudPS;
	ComPtr<ID3D11PixelShader> samplingPS;
	ComPtr<ID3D11PixelShader> instancePS;
	ComPtr<ID3D11PixelShader> postEffectPS;
	ComPtr<ID3D11PixelShader> mirrorMaskingPS;


	// Rasterizer State
	ComPtr<ID3D11RasterizerState> solidRS;
	ComPtr<ID3D11RasterizerState> wireRS;
	ComPtr<ID3D11RasterizerState> noneCullRS;


	// Sampler State
	ComPtr<ID3D11SamplerState> pointWrapSS;
	ComPtr<ID3D11SamplerState> linearWrapSS;
	ComPtr<ID3D11SamplerState> linearClampSS;


	// Depth Stencil State
	ComPtr<ID3D11DepthStencilState> basicDSS;
	ComPtr<ID3D11DepthStencilState> postEffectDSS;
	ComPtr<ID3D11DepthStencilState> mirrorMaskingDSS;


	// Blend State
	ComPtr<ID3D11BlendState> alphaBS;


	// RTV & Buffer
	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	ComPtr<ID3D11Texture2D> basicRenderBuffer;
	ComPtr<ID3D11RenderTargetView> basicRTV;

	ComPtr<ID3D11Texture2D> cloudRenderBuffer;
	ComPtr<ID3D11RenderTargetView> cloudRTV;

	ComPtr<ID3D11Texture2D> postEffectBuffer;
	ComPtr<ID3D11RenderTargetView> postEffectRTV;

	ComPtr<ID3D11Texture2D> envMapRenderBuffer;
	ComPtr<ID3D11RenderTargetView> envMapRTV;

	ComPtr<ID3D11Texture2D> mirrorWorldRenderBuffer;
	ComPtr<ID3D11RenderTargetView> mirrorWorldRTV;


	// DSV & Buffer
	ComPtr<ID3D11Texture2D> basicDepthBuffer;
	ComPtr<ID3D11DepthStencilView> basicDSV;

	ComPtr<ID3D11Texture2D> depthOnlyBuffer;
	ComPtr<ID3D11DepthStencilView> depthOnlyDSV;

	ComPtr<ID3D11Texture2D> envMapDepthBuffer;
	ComPtr<ID3D11DepthStencilView> envMapDSV;

	ComPtr<ID3D11Texture2D> mirrorWorldDepthBuffer;
	ComPtr<ID3D11DepthStencilView> mirrorWorldDSV;

	ComPtr<ID3D11Texture2D> mirrorPlaneDepthBuffer;
	ComPtr<ID3D11DepthStencilView> mirrorPlaneDSV;


	// SRV & Buffer
	ComPtr<ID3D11Texture2D> atlasMapBuffer;
	ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	ComPtr<ID3D11Texture2D> sunBuffer;
	ComPtr<ID3D11ShaderResourceView> sunSRV;
	ComPtr<ID3D11Texture2D> moonBuffer;
	ComPtr<ID3D11ShaderResourceView> moonSRV;

	ComPtr<ID3D11Texture2D> cloudResolvedBuffer;
	ComPtr<ID3D11ShaderResourceView> cloudSRV;

	ComPtr<ID3D11ShaderResourceView> depthOnlySRV;

	ComPtr<ID3D11Texture2D> postEffectResolvedBuffer;
	ComPtr<ID3D11ShaderResourceView> postEffectSRV;

	ComPtr<ID3D11ShaderResourceView> envMapSRV;

	ComPtr<ID3D11ShaderResourceView> mirrorWorldRenderSRV;
	ComPtr<ID3D11ShaderResourceView> mirrorPlaneDepthSRV;


	// Viewport
	D3D11_VIEWPORT basicViewport;
	D3D11_VIEWPORT envMapViewPort;
	D3D11_VIEWPORT mirrorWorldViewPort;


	// PSO
	void InitGraphicsPSO();
	void SetPipelineStates(GraphicsPSO& pso);
	GraphicsPSO basicPSO;
	GraphicsPSO basicWirePSO;
	GraphicsPSO basicNoneCullPSO;
	GraphicsPSO skyboxPSO;
	GraphicsPSO skyboxEnvMapPSO;
	GraphicsPSO cloudPSO;
	GraphicsPSO cloudBlendPSO;
	GraphicsPSO postEffectPSO;
	GraphicsPSO instancePSO;
	GraphicsPSO mirrorMaskingPSO;
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
		D3D_FEATURE_LEVEL_11_1,
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
	desc.SampleDesc.Count = 1; // backbuffer
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

	if (!InitShaderResourceBuffers(width, height))
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
	UINT bindFlag = D3D11_BIND_RENDER_TARGET;
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


	// Cloud RTV
	if (!DXUtils::CreateTextureBuffer(cloudRenderBuffer, width, height, true, format, bindFlag)) {
		std::cout << "failed create render target buffer" << std::endl;
		return false;
	}
	ret = Graphics::device->CreateRenderTargetView(
		cloudRenderBuffer.Get(), nullptr, cloudRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create render target view" << std::endl;
		return false;
	}


	// EnvMap RTV
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	UINT miscFlag = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (!DXUtils::CreateTextureBuffer(
			envMapRenderBuffer, width / 8, width / 8, false, format, bindFlag, 1, 6, miscFlag)) {
		std::cout << "failed create env map buffer" << std::endl;
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 6;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	ret = Graphics::device->CreateRenderTargetView(
		envMapRenderBuffer.Get(), &rtvDesc, envMapRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create env target view" << std::endl;
		return false;
	}


	// MirrorWorld RTV
	if (!DXUtils::CreateTextureBuffer(
			mirrorWorldRenderBuffer, width / 4, height / 4, false, format, bindFlag)) {
		std::cout << "failed create mirror world render buffer" << std::endl;
		return false;
	}

	ret = Graphics::device->CreateRenderTargetView(
		mirrorWorldRenderBuffer.Get(), nullptr, mirrorWorldRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror world rtv" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitDepthStencilBuffers(UINT width, UINT height)
{
	// basic DSV
	DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT bindFlag = D3D11_BIND_DEPTH_STENCIL;
	if (!DXUtils::CreateTextureBuffer(basicDepthBuffer, width, height, true, format, bindFlag)) {
		std::cout << "failed create depth stencil buffer" << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	HRESULT ret = Graphics::device->CreateDepthStencilView(
		basicDepthBuffer.Get(), &dsvDesc, basicDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create depth stencil view" << std::endl;
		return false;
	}


	// depthOnly
	format = DXGI_FORMAT_R32_TYPELESS;
	bindFlag = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(depthOnlyBuffer, width, height, false, format, bindFlag)) {
		std::cout << "failed create depth stencil buffer" << std::endl;
		return false;
	}

	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ret = Graphics::device->CreateDepthStencilView(
		depthOnlyBuffer.Get(), &dsvDesc, depthOnlyDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create depth stencil view" << std::endl;
		return false;
	}


	// envMap
	format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bindFlag = D3D11_BIND_DEPTH_STENCIL;
	UINT miscFlag = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (!DXUtils::CreateTextureBuffer(
			envMapDepthBuffer, width / 8, width / 8, false, format, bindFlag, 1, 6, miscFlag)) {
		std::cout << "failed create env map depth stencil buffer" << std::endl;
		return false;
	}

	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 6;
	dsvDesc.Texture2DArray.FirstArraySlice = 0;
	ret = Graphics::device->CreateDepthStencilView(
		envMapDepthBuffer.Get(), &dsvDesc, envMapDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create env map depth stencil view" << std::endl;
		return false;
	}


	// mirrorWorld
	if (!DXUtils::CreateTextureBuffer(
			mirrorWorldDepthBuffer, width / 4, height / 4, false, format, bindFlag)) {
		std::cout << "failed create mirror world depth stencil buffer" << std::endl;
		return false;
	}

	ret = Graphics::device->CreateDepthStencilView(
		mirrorWorldDepthBuffer.Get(), nullptr, mirrorWorldDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror world depth stencil view" << std::endl;
		return false;
	}


	// mirrorPlane
	format = DXGI_FORMAT_R24G8_TYPELESS;
	bindFlag = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			mirrorPlaneDepthBuffer, width / 4, height / 4, false, format, bindFlag)) {
		std::cout << "failed create mirror plane depth stencil buffer" << std::endl;
		return false;
	}

	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ret = Graphics::device->CreateDepthStencilView(
		mirrorPlaneDepthBuffer.Get(), &dsvDesc, mirrorPlaneDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror plane depth stencil view" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitShaderResourceBuffers(UINT width, UINT height)
{
	// Asset Files
	if (!DXUtils::CreateTextureArrayFromAtlasFile(
			atlasMapBuffer, atlasMapSRV, "../assets/blockatlas1.png")) {
		std::cout << "failed create texture from atlas file" << std::endl;
		return false;
	}

	/*if (!DXUtils::CreateTextureFromFile(
			grassColorMapBuffer, grassColorMapSRV, "../assets/grass_color_map.png")) {
		std::cout << "failed create texture from grass color map file" << std::endl;
		return false;
	}*/

	if (!DXUtils::CreateTexture2DFromFile(sunBuffer, sunSRV, "../assets/sun.png")) {
		std::cout << "failed create texture from sun file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTexture2DFromFile(moonBuffer, moonSRV, "../assets/moon.png")) {
		std::cout << "failed create texture from moon file" << std::endl;
		return false;
	}


	// cloudSRV
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT bindFlag = D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			cloudResolvedBuffer, width, height, false, format, bindFlag)) {
		std::cout << "failed create shader resource buffer" << std::endl;
		return false;
	}
	HRESULT ret = Graphics::device->CreateShaderResourceView(
		cloudResolvedBuffer.Get(), 0, cloudSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from cloud srv" << std::endl;
		return false;
	}


	// depthOnly
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	ret = Graphics::device->CreateShaderResourceView(
		Graphics::depthOnlyBuffer.Get(), &srvDesc, depthOnlySRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from depthOlny srv" << std::endl;
		return false;
	}


	// postEffectSRV
	format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bindFlag = D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			postEffectResolvedBuffer, width, height, false, format, bindFlag)) {
		std::cout << "failed create shader resource buffer" << std::endl;
		return false;
	}
	ret = Graphics::device->CreateShaderResourceView(
		postEffectResolvedBuffer.Get(), 0, postEffectSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from cloud srv" << std::endl;
		return false;
	}


	// envMapSRV
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	ret = device->CreateShaderResourceView(
		envMapRenderBuffer.Get(), &srvDesc, envMapSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from env map srv" << std::endl;
		return false;
	}


	// mirror world render SRV
	ret = Graphics::device->CreateShaderResourceView(
		mirrorWorldRenderBuffer.Get(), 0, mirrorWorldRenderSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from mirror world render srv" << std::endl;
		return false;
	}
	

	// mirror plane depth SRV
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	ret = Graphics::device->CreateShaderResourceView(
		Graphics::mirrorPlaneDepthBuffer.Get(), &srvDesc, mirrorPlaneDepthSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from mirror plane depth srv" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGraphicsState()
{
	if (!InitVertexShaderAndInputLayouts())
		return false;

	if (!InitGeometryShaders())
		return false;

	if (!InitPixelShaders())
		return false;

	if (!InitRasterizerStates())
		return false;

	if (!InitSamplerStates())
		return false;

	if (!InitDepthStencilStates())
		return false;

	if (!InitBlendStates())
		return false;

	return true;
}

bool Graphics::InitVertexShaderAndInputLayouts()
{
	// BasicVS & BasicIL
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc = {
		{ "DATA", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"BasicVS.hlsl", basicVS, basicIL, elementDesc)) {
		std::cout << "failed create basic vs" << std::endl;
		return false;
	}

	// SkyBox
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc2 = { { "POSITION", 0,
		DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"SkyboxVS.hlsl", skyboxVS, skyboxIL, elementDesc2)) {
		std::cout << "failed create skybox vs" << std::endl;
		return false;
	}

	// Skybox EnvMap
	std::vector<D3D_SHADER_MACRO> macros = { { "USE_INPUT_SKYBOX", "1" }, { NULL, NULL } };
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"EnvMapVS.hlsl", skyboxEnvMapVS, skyboxIL, elementDesc2, macros.data())) {
		std::cout << "failed create skybox env map vs" << std::endl;
		return false;
	}

	// Cloud
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc3 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "FACE", 0, DXGI_FORMAT_R8_UINT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"CloudVS.hlsl", cloudVS, cloudIL, elementDesc3)) {
		std::cout << "failed create cloud vs" << std::endl;
		return false;
	}

	// Sampling
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc4 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"SamplingVS.hlsl", samplingVS, samplingIL, elementDesc4)) {
		std::cout << "failed create sampling vs" << std::endl;
		return false;
	}

	// Instance
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc6 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"InstanceVS.hlsl", instanceVS, instanceIL, elementDesc6)) {
		std::cout << "failed create instance vs" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGeometryShaders()
{
	// Skybox EnvMap
	std::vector<D3D_SHADER_MACRO> macros = { { "USE_INPUT_SKYBOX", "1" }, { NULL, NULL } };
	if (!DXUtils::CreateGeometryShader(L"EnvMapGS.hlsl", skyboxEnvMapGS, macros.data())) {
		std::cout << "failed create skybox env map gs" << std::endl;
		return false;
	}
	return true;
}

bool Graphics::InitPixelShaders()
{
	// BasicPS
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicPS)) {
		std::cout << "failed create basic ps" << std::endl;
		return false;
	}

	// SkyboxPS
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxPS)) {
		std::cout << "failed create skybox ps" << std::endl;
		return false;
	}

	// Skybox EnvMap
	std::vector<D3D_SHADER_MACRO> macros = { { "USE_RENDER_TARGET_ARRAY_INDEX", "1" },
		{ NULL, NULL } };
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxEnvMapPS, macros.data())) {
		std::cout << "failed create skybox env map ps" << std::endl;
		return false;
	}

	// CloudPS
	if (!DXUtils::CreatePixelShader(L"CloudPS.hlsl", cloudPS)) {
		std::cout << "failed create cloud ps" << std::endl;
		return false;
	}

	// SamplingPS
	if (!DXUtils::CreatePixelShader(L"SamplingPS.hlsl", samplingPS)) {
		std::cout << "failed create sampling ps" << std::endl;
		return false;
	}

	// PostEffectPS
	if (!DXUtils::CreatePixelShader(L"PostEffectPS.hlsl", postEffectPS)) {
		std::cout << "failed create PostEffect ps" << std::endl;
		return false;
	}

	// InstancePS
	if (!DXUtils::CreatePixelShader(L"InstancePS.hlsl", instancePS)) {
		std::cout << "failed create instance ps" << std::endl;
		return false;
	}

	// MirrorMaskingPS
	if (!DXUtils::CreatePixelShader(L"MirrorMaskingPS.hlsl", mirrorMaskingPS)) {
		std::cout << "failed create mirrorMasking ps" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitRasterizerStates()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
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


	// noneCullRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, noneCullRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create noneCull RS" << std::endl;
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

	// point wrap
	desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	HRESULT ret = Graphics::device->CreateSamplerState(&desc, pointWrapSS.GetAddressOf());
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

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ret = Graphics::device->CreateSamplerState(&desc, linearClampSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create linear Clamp SS" << std::endl;
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
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	// basic DSS
	HRESULT ret = Graphics::device->CreateDepthStencilState(&desc, basicDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic DSS" << std::endl;
		return false;
	}

	// PostEffect DSS
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	ret = Graphics::device->CreateDepthStencilState(&desc, postEffectDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create postEffect DSS" << std::endl;
		return false;
	}
	
	// Mirror Masking DSS
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // stencil X
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // stencil O depth X
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // stencil O depth O
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	ret = Graphics::device->CreateDepthStencilState(&desc, mirrorMaskingDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror masking DSS" << std::endl;
		return false;
	}
	
	return true;
}

bool Graphics::InitBlendStates()
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT ret = Graphics::device->CreateBlendState(&desc, alphaBS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create alpha BS" << std::endl;
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
	basicPSO.geometryShader = nullptr;
	basicPSO.rasterizerState = solidRS;
	basicPSO.pixelShader = basicPS;
	basicPSO.samplerStates.push_back(pointWrapSS.Get());
	basicPSO.samplerStates.push_back(linearWrapSS.Get());
	basicPSO.samplerStates.push_back(linearClampSS.Get());
	basicPSO.depthStencilState = basicDSS;
	basicPSO.stencilRef = 0;
	basicPSO.blendState = nullptr;

	// basic wire PSO
	basicWirePSO = basicPSO;
	basicWirePSO.rasterizerState = wireRS;

	// basic none cull PSO
	basicNoneCullPSO = basicPSO;
	basicNoneCullPSO.rasterizerState = noneCullRS;

	// skyboxPSO
	skyboxPSO = basicPSO;
	skyboxPSO.inputLayout = skyboxIL;
	skyboxPSO.vertexShader = skyboxVS;
	skyboxPSO.pixelShader = skyboxPS;

	// skyboxEnvMapPSO
	skyboxEnvMapPSO = skyboxPSO;
	skyboxEnvMapPSO.vertexShader = skyboxEnvMapVS;
	skyboxEnvMapPSO.geometryShader = skyboxEnvMapGS;
	skyboxEnvMapPSO.pixelShader = skyboxEnvMapPS;

	// cloudPSO
	cloudPSO = basicPSO;
	cloudPSO.inputLayout = cloudIL;
	cloudPSO.vertexShader = cloudVS;
	cloudPSO.pixelShader = cloudPS;
	cloudPSO.blendState = alphaBS;

	// postEffectPSO
	postEffectPSO = basicPSO;
	postEffectPSO.vertexShader = samplingVS;
	postEffectPSO.pixelShader = postEffectPS;
	postEffectPSO.inputLayout = samplingIL;

	// instancePSO
	instancePSO = basicPSO;
	instancePSO.inputLayout = instanceIL;
	instancePSO.vertexShader = instanceVS;
	instancePSO.rasterizerState = noneCullRS;
	instancePSO.pixelShader = instancePS;

	// mirrorMaskingPSO
	mirrorMaskingPSO = basicPSO;
	mirrorMaskingPSO.depthStencilState = mirrorMaskingDSS;
	mirrorMaskingPSO.stencilRef = 1;
	mirrorMaskingPSO.pixelShader = mirrorMaskingPS;
}

void Graphics::SetPipelineStates(GraphicsPSO& pso)
{
	context->IASetInputLayout(pso.inputLayout.Get());
	context->IASetPrimitiveTopology(pso.topology);

	context->VSSetShader(pso.vertexShader.Get(), nullptr, 0);

	context->GSSetShader(pso.geometryShader.Get(), nullptr, 0);

	context->RSSetState(pso.rasterizerState.Get());

	context->PSSetShader(pso.pixelShader.Get(), nullptr, 0);

	if (pso.samplerStates.empty())
		context->PSSetSamplers(0, 0, nullptr);
	else
		context->PSSetSamplers(0, (UINT)pso.samplerStates.size(), pso.samplerStates.data());

	context->OMSetDepthStencilState(pso.depthStencilState.Get(), pso.stencilRef);

	context->OMSetBlendState(pso.blendState.Get(), pso.blendFactor, 0xffffffff);
}