#include "Utils.h"

bool Utils::CreateDeviceAndSwapChain(ComPtr<ID3D11Device>& device,
	ComPtr<ID3D11DeviceContext>& context, ComPtr<IDXGISwapChain>& swapChain, HWND& hwnd, UINT width,
	UINT height)
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
	if (FAILED(ret))
		return false; 

	UINT qualityLevel = 0;
	ret =
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevel);
	if (FAILED(ret))
		return false;

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (qualityLevel == 0) {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	else {
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Quality = qualityLevel - 1;
	}
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.OutputWindow = hwnd;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ret = D3D11CreateDeviceAndSwapChain(NULL, driverType, 0, deviceFlags, levels,
		ARRAYSIZE(levels), D3D11_SDK_VERSION, &desc, &swapChain,
		&device, &featureLevel, &context);

	if (FAILED(ret))
		return false;
	return true;
}

bool Utils::CreateVertexShader(ComPtr<ID3D11Device>& device, const std::wstring& filename,
	ComPtr<ID3D11VertexShader>& vs, ComPtr<ID3D11InputLayout>& il)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT ret = D3DCompileFromFile(
		filename.c_str(), 0, 0, "main", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
	if (FAILED(ret)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		if (shaderBlob)
			shaderBlob->Release();

		return false;
	}

	ret = device->CreateVertexShader(
		shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, vs.GetAddressOf());
	if (FAILED(ret))
		return false;

	std::vector<D3D11_INPUT_ELEMENT_DESC> pDesc = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4*3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4*3*2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4*3*3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	ret = device->CreateInputLayout(pDesc.data(), (UINT)pDesc.size(),
		shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), il.GetAddressOf());
	if (FAILED(ret))
		return false;
	return true;
}

bool Utils::CreatePixelShader(
	ComPtr<ID3D11Device>& device, const std::wstring& filename, ComPtr<ID3D11PixelShader>& ps)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT ret = D3DCompileFromFile(
		filename.c_str(), 0, 0, "main", "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
	if (FAILED(ret)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		if (shaderBlob)
			shaderBlob->Release();

		return false;
	}

	ret = device->CreatePixelShader(
		shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, ps.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateRasterizerState(ComPtr<ID3D11Device>& device, ComPtr<ID3D11RasterizerState>& rs)
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC)); // Need this
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	// rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true; // <- zNear, zFar 확인에 필요
	rastDesc.MultisampleEnable = true;
	
	HRESULT ret = device->CreateRasterizerState(&rastDesc, rs.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

void Utils::UpdateViewport(
	D3D11_VIEWPORT& viewport, FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height)
{
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = topLeftX;
	viewport.TopLeftY = topLeftY;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}


bool Utils::CreateRenderTargetBuffer(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& rt, UINT width, UINT height)
{
	UINT qualityLevel = 0;
	HRESULT ret =
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevel);
	if (FAILED(ret))
		return false;


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (qualityLevel == 0) {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	else {
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Quality = qualityLevel - 1;
	}
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	ret = device->CreateTexture2D(&desc, nullptr, rt.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateRenderTargetView(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& rt, ComPtr<ID3D11RenderTargetView>& rtv)
{
	HRESULT ret = device->CreateRenderTargetView(rt.Get(), nullptr, rtv.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}
#include <iostream>
bool Utils::CreateDepthStencilBuffer(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& ds, UINT width, UINT height)
{
	UINT qualityLevel = 0;
	HRESULT ret =
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, 4, &qualityLevel);
	if (FAILED(ret))
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (qualityLevel == 0) {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	else {
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Quality = qualityLevel - 1;
	}
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ret = device->CreateTexture2D(&desc, nullptr, ds.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateDepthStencilView(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& ds, ComPtr<ID3D11DepthStencilView>& dsv)
{
	HRESULT ret = device->CreateDepthStencilView(ds.Get(), nullptr, dsv.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateDepthStencilState(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11DepthStencilState>& dss)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;

	HRESULT ret = device->CreateDepthStencilState(&desc, dss.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateVertexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& vertexBuffer, std::vector<Vertex>& vertices, UINT& stride, UINT& offset) {
	stride = sizeof(Vertex);
	offset = 0;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = vertices.data();

	HRESULT ret = device->CreateBuffer(&desc, &data, vertexBuffer.GetAddressOf());
	if (FAILED(ret))
		return false;
	return true;
}

bool Utils::CreateIndexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& indexBuffer,
	std::vector<uint32_t>& indices)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = indices.data();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT ret = device->CreateBuffer(&desc, &data, indexBuffer.GetAddressOf());
	if (FAILED(ret))
		return false;
	return true;
}

bool Utils::CreateConstantBuffer(ComPtr<ID3D11Device>& device,
	ComPtr<ID3D11Buffer>& constantBuffer,
	GlobalConstantData& constantData)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(GlobalConstantData);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &constantData;

	HRESULT ret = device->CreateBuffer(&desc, &data, constantBuffer.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateConstantBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& constantBuffer,
	ChunkConstantData& constantData)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(ChunkConstantData);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &constantData;

	HRESULT ret = device->CreateBuffer(&desc, &data, constantBuffer.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

void Utils::UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context,
	ComPtr<ID3D11Buffer>& constantBuffer, GlobalConstantData& constantData)
{
	D3D11_MAPPED_SUBRESOURCE ms;

	context->Map(constantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &constantData, sizeof(constantData));
	context->Unmap(constantBuffer.Get(), NULL);
}

void Utils::UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context,
	ComPtr<ID3D11Buffer>& constantBuffer, ChunkConstantData& constantData)
{
	D3D11_MAPPED_SUBRESOURCE ms;

	context->Map(constantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &constantData, sizeof(constantData));
	context->Unmap(constantBuffer.Get(), NULL);
}