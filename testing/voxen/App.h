#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <iostream>
#include <windows.h>
#include <wrl.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <thread>
#include <future>
#include <vector>

#include "Utils.h"
#include "Chunk.h"
#include "Camera.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::SimpleMath;



class App
{
public:
	App();
	~App();

	LRESULT EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Initialize();
	void Run();


private:
	

	void Update(float dt);
	void Render();

	bool InitWindow();
	bool InitDirectX();
	bool InitGUI();
	void InitMesh();
	
	
	HWND m_hwnd;
	UINT m_width;
	UINT m_height;

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;

	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	
	ComPtr<ID3D11RasterizerState> m_rasterizerState;
	D3D11_VIEWPORT m_viewport;

	ComPtr<ID3D11Texture2D> m_renderTargetBuffer;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	ComPtr<ID3D11Texture2D> m_backBuffer;
	ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

	ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	ComPtr<ID3D11PixelShader> m_pixelShader;

	GlobalConstantData m_globalConstantData;
	ComPtr<ID3D11Buffer> m_globalConstantBuffer;

	Chunk ***m_chunks;
	static const int CHUNK_SIZE = 4;

	Camera m_camera;

	bool m_keyPressed[256];
	float m_mouseNdcX;
	float m_mouseNdcY;

	ComPtr<ID3D11Texture2D> m_texture;
	ComPtr<ID3D11ShaderResourceView> m_textureSRV1;
	ComPtr<ID3D11ShaderResourceView> m_textureSRV2;
	ComPtr<ID3D11ShaderResourceView> m_textureSRV3;
	ComPtr<ID3D11ShaderResourceView> m_textureSRV4;

	ComPtr<ID3D11SamplerState> m_samplerState;
};


