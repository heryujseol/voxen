#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <vector>
#include <fstream>

#include "Block.h"


using namespace Microsoft::WRL;

struct GlobalConstantData {
	Matrix view;
	Matrix proj;
};

struct ChunkConstantData {
	Matrix world;
	Matrix invTrans;
};

namespace Utils {
	extern bool CreateDeviceAndSwapChain(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context, ComPtr<IDXGISwapChain>& swapChain, HWND& hwnd,
		UINT width, UINT height);

	extern bool CreateVertexShader(ComPtr<ID3D11Device>& device, const std::wstring& filename,
		ComPtr<ID3D11VertexShader>& vs, ComPtr<ID3D11InputLayout>& il);

	extern bool CreatePixelShader(
		ComPtr<ID3D11Device>& device, const std::wstring& filename, ComPtr<ID3D11PixelShader>& ps);

	extern bool CreateRasterizerState(
		ComPtr<ID3D11Device>& device, ComPtr<ID3D11RasterizerState>& rs);

	extern void UpdateViewport(
		D3D11_VIEWPORT& viewport, FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height);

	extern bool CreateRenderTargetBuffer(
		ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& rt, UINT width, UINT height);

	extern bool CreateRenderTargetView(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& rt,
		ComPtr<ID3D11RenderTargetView>& rtv);

	extern bool CreateDepthStencilBuffer(
		ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& ds, UINT width, UINT height);

	extern bool CreateDepthStencilView(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Texture2D>& ds,
		ComPtr<ID3D11DepthStencilView>& dsv);

	extern bool CreateDepthStencilState(
		ComPtr<ID3D11Device>& device, ComPtr<ID3D11DepthStencilState>& dss);

	extern bool CreateVertexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& vertexBuffer,
		std::vector<Vertex>& vertices, UINT& stride, UINT& offset);

	extern bool CreateIndexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11Buffer>& indexBuffer,
		std::vector<uint32_t>& indices);

	extern bool CreateConstantBuffer(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11Buffer>& constantBuffer, GlobalConstantData& constantData);

	extern bool CreateConstantBuffer(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11Buffer>& constantBuffer, ChunkConstantData& constantData);

	extern void UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context,
		ComPtr<ID3D11Buffer>& constantBuffer, GlobalConstantData& constantData);

	extern void UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context,
		ComPtr<ID3D11Buffer>& constantBuffer, ChunkConstantData& constantData);

}

/*
D3D11_MAPPED_SUBRESOURCE ms;

		m_context->Map(m_globalConstantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_globalConstantData, sizeof(m_globalConstantData));
		m_context->Unmap(m_globalConstantBuffer.Get(), NULL);
*/