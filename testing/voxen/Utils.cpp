#include "Utils.h"

float Utils::Lerp(float a, float b, float w) { return (1 - w) * a + w * b; }

float Utils::CubicLerp(float a, float b, float w)
{
	return (b - a) * (3.0f - w * 2.0f) * w * w + a;
}

float Utils::Smootherstep(float a, float b, float w)
{
	return (b - a) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a;
}

Vector2 Utils::RandomGradient(int ix, int iy)
{
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;

	a *= 3284157443;
	b ^= a << s | a >> w - s;
	b *= 1911520717;
	a ^= b << s | b >> w - s;
	a *= 2048419325;

	float random = a * ((float)3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	Vector2 v(cos(random), sin(random));
	return v;
}

float Utils::GetPerlinNoise(float x, float y)
{
	Vector2 p = Vector2(x, y);
	int x0 = (int)floor(x);
	int x1 = x0 + 1;
	int y0 = (int)floor(y);
	int y1 = y0 + 1;

	float n0 = RandomGradient(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
	float n1 = RandomGradient(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
	float n2 = RandomGradient(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
	float n3 = RandomGradient(x1, y1).Dot(p - Vector2((float)x1, (float)y1));

	float inter_x0 = CubicLerp(n0, n1, p.x - (float)x0);
	float inter_x1 = CubicLerp(n2, n3, p.x - (float)x0);
	float inter_y = CubicLerp(inter_x0, inter_x1, p.y - (float)y0);
	return inter_y;
}

int Utils::GetHeight(int x, int z)
{
	float noise = GetPerlinNoise((float)x / 32.0f, (float)z / 32.0f); // [-1.0, 1.0]

	return (int)((noise + 1.0f) * 24.0f); // [0, 48]
}

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
	ret = device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevel);
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

	ret = D3D11CreateDeviceAndSwapChain(NULL, driverType, 0, deviceFlags, levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION, &desc, &swapChain, &device, &featureLevel, &context);

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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

bool Utils::CreateVertexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& vertexBuffer,
	std::vector<Vertex>& vertices, UINT& stride, UINT& offset)
{
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

bool Utils::CreateIndexBuffer(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& indexBuffer, std::vector<uint32_t>& indices)
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

bool Utils::CreateConstantBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& constantBuffer,
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

bool Utils::CreateTexture2DFromFile(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& texture, std::string filename)
{
	int width, height, channel = 4;
	std::vector<uint8_t> image;
	ReadImage(filename, image, width, height);

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = image.data();
	data.SysMemPitch = UINT(width * sizeof(uint8_t) * channel);

	HRESULT ret = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateShaderResourceView(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& texture,
	ComPtr<ID3D11ShaderResourceView>& srv)
{
	HRESULT ret = device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

bool Utils::CreateSamplerState(
	ComPtr<ID3D11Device>& device, ComPtr<ID3D11SamplerState>& samplerState)
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	HRESULT ret = device->CreateSamplerState(&desc, samplerState.GetAddressOf());
	if (FAILED(ret))
		return false;

	return true;
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
void Utils::ReadImage(
	const std::string filename, std::vector<uint8_t>& image, int& width, int& height)
{
	int channels = 4;
	unsigned char* img = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	image.resize((size_t)width * height * 4);
	

	if (channels == 1) {
		for (size_t i = 0; i < (size_t)width * height; i++) {
			uint8_t g = img[i * channels + 0];
			for (size_t c = 0; c < 4; c++) {
				image[4 * i + c] = g;
			}
		}
	}
	else if (channels == 2) {
		for (size_t i = 0; i < (size_t)width * height; i++) {
			for (size_t c = 0; c < 2; c++) {
				image[4 * i + c] = img[i * channels + c];
			}
			image[4 * i + 2] = 255;
			image[4 * i + 3] = 255;
		}
	}
	else if (channels == 3) {
		for (size_t i = 0; i < (size_t)width * height; i++) {
			for (size_t c = 0; c < 3; c++) {
				image[4 * i + c] = img[i * channels + c];
			}
			image[4 * i + 3] = 255;
		}
	}
	else if (channels == 4) {
		for (size_t i = 0; i < (size_t)width * height; i++) {
			for (size_t c = 0; c < 4; c++) {
				image[4 * i + c] = img[i * channels + c];
			}
		}
	}
	else {
		std::cout << "Cannot read " << channels << " channels" << std::endl;
	}

	delete[] img;
}