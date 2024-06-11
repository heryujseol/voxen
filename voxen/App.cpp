#include "App.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <iostream>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>


App* g_app = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return g_app->EventHandler(hwnd, uMsg, wParam, lParam);
}

App::App()
	: m_width(1920), m_height(1080), m_hwnd(), m_chunkManager(), m_camera(), m_skybox(),
	  m_mouseNdcX(0.0f), m_mouseNdcY(0.0f),
	  m_keyPressed{
		  false,
	  },
	  m_keyToggle{
		  false,
	  }
{
	g_app = this;
}

App::~App()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(m_hwnd);
}

LRESULT App::EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (UINT(wParam) == VK_ESCAPE) {
			DestroyWindow(hwnd);
			return 0;
		}
		m_keyPressed[UINT(wParam)] = true;
		m_keyToggle[UINT(wParam)] = !m_keyToggle[UINT(wParam)];
		break;

	case WM_KEYUP:
		m_keyPressed[UINT(wParam)] = false;
		break;

	case WM_MOUSEMOVE:
		m_mouseNdcX = (float)LOWORD(lParam) / (float)m_width * 2 - 1;
		m_mouseNdcY = -((float)HIWORD(lParam) / (float)m_height * 2 - 1);

		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool App::Initialize()
{
	if (!InitWindow())
		return false;

	if (!InitDirectX())
		return false;

	if (!InitGUI())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void App::Run()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			ImGui_ImplDX11_NewFrame(); // GUI 프레임 시작
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame(); // 어떤 것들을 렌더링 할지 기록 시작
			ImGui::Begin("Scene Control");
			ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);
			Vector3 sun = m_skybox.GetSun();
			ImGui::Text("SunDir : %.2f %.2f", sun.x, sun.y);
			uint32_t time = m_skybox.GetTime();
			ImGui::Text("time : %d", time);
			ImGui::Text("x : %.0f y : %.0f z : %.0f", m_camera.GetPosition().x,
				m_camera.GetPosition().y, m_camera.GetPosition().z);
			ImGui::End();
			ImGui::Render(); // 렌더링할 것들 기록 끝

			Update(ImGui::GetIO().DeltaTime);
			Render(); // 우리가 구현한 렌더링

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // GUI 렌더링

			Graphics::swapChain->Present(1, 0);
		}
	}
}

void App::Update(float dt)
{
	m_camera.Update(dt, m_keyPressed, m_mouseNdcX, m_mouseNdcY);
	m_chunkManager.Update(m_camera);
	if (m_keyToggle['F']) {
		m_skybox.Update(dt);
		m_cloud.Update(dt, m_camera.GetPosition());
	}
	else {
		m_skybox.Update(0.0f);
		m_cloud.Update(0.0f, m_camera.GetPosition());
	}
}

void App::Render()
{
	// 공통 로직
	DXUtils::UpdateViewport(Graphics::basicViewport, 0, 0, m_width, m_height);
	Graphics::context->RSSetViewports(1, &Graphics::basicViewport);

	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	Graphics::context->VSSetConstantBuffers(0, 1, m_camera.m_constantBuffer.GetAddressOf());
	std::vector<ID3D11Buffer*> pptr = { m_camera.m_constantBuffer.Get(),
		m_skybox.m_constantBuffer.Get() };
	Graphics::context->PSSetConstantBuffers(0, 2, pptr.data());


	//depthMap
	DepthMapRender();

	Graphics::context->ClearRenderTargetView(Graphics::basicRTV.Get(), clearColor);
	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicRTV.GetAddressOf(), Graphics::basicDSV.Get());
	Graphics::context->ClearDepthStencilView(
		Graphics::basicDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	// basic
	Graphics::SetPipelineStates(m_keyToggle[9] ? Graphics::basicWirePSO : Graphics::basicPSO);
	m_chunkManager.RenderOpaque();
	if (!m_keyToggle['L'])
		m_chunkManager.RenderSemiAlpha();


	// instance
	Graphics::SetPipelineStates(Graphics::instancePSO);
	m_chunkManager.RenderInstance();
	

	// postEffect
	Graphics::SetPipelineStates(Graphics::postEffectPSO);
	m_postEffect.Render();


	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicRTV.GetAddressOf(), Graphics::basicDSV.Get());

	// skybox
	Graphics::SetPipelineStates(Graphics::skyboxPSO);
	m_skybox.Render();


	// cloud
	Graphics::SetPipelineStates(Graphics::cloudPSO);
	m_cloud.Render();

	
	// RTV -> backBuffer
	Graphics::context->ResolveSubresource(Graphics::backBuffer.Get(), 0,
		Graphics::basicRenderBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	// GUI 렌더링을 위한 RTV 재설정
	Graphics::context->OMSetRenderTargets(1, Graphics::backBufferRTV.GetAddressOf(), nullptr);
}

bool App::InitWindow()
{
	const wchar_t CLASS_NAME[] = L"Voxen Class";
	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL),
		NULL, NULL, NULL, NULL,
		CLASS_NAME, // lpszClassName, L-string
		NULL };

	if (!RegisterClassEx(&wc))
		return false;


	RECT wr = { 0, 0, (LONG)m_width, (LONG)m_height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindow(wc.lpszClassName, L"Voxen", WS_OVERLAPPEDWINDOW, 50, 50,
		wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (m_hwnd == NULL)
		return false;

	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);

	return true;
}

bool App::InitDirectX()
{
	DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (!Graphics::InitGraphicsCore(pixelFormat, m_hwnd, m_width, m_height)) {
		return false;
	}

	if (!Graphics::InitGraphicsBuffer(m_width, m_height)) {
		return false;
	}

	if (!Graphics::InitGraphicsState()) {
		return false;
	}

	Graphics::InitGraphicsPSO();

	return true;
}

bool App::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.DisplaySize = ImVec2(float(m_width), float(m_height));
	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplDX11_Init(Graphics::device.Get(), Graphics::context.Get())) {
		return false;
	}

	if (!ImGui_ImplWin32_Init(m_hwnd)) {
		return false;
	}

	return true;
}

bool App::InitScene()
{
	if (!m_camera.Initialize(Vector3(0.0f, 108.0f, 0.0f)))
		return false;

	if (!m_chunkManager.Initialize(m_camera.GetChunkPosition()))
		return false;

	if (!m_skybox.Initialize(550.0f, 0.2f))
		return false;

	if (!m_cloud.Initialize(m_camera.GetPosition()))
		return false;

	if (!m_postEffect.Initialize())
		return false;

	return true;
}

void App::DepthMapRender()
{
	Graphics::context->OMSetRenderTargets(0, NULL, Graphics::depthOnlyDSV.Get());
	Graphics::context->ClearDepthStencilView(
		Graphics::depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	Graphics::SetPipelineStates(Graphics::basicPSO);
	m_chunkManager.RenderOpaque();
	if (!m_keyToggle['L'])
		m_chunkManager.RenderSemiAlpha();
}