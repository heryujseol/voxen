#include "App.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

App::App() : m_width(1600), m_height(900), m_hwnd(), m_viewport(), m_chunks(nullptr)
{
	Vector3 eyepos = Vector3(320.0f, 8.0f, -328.0f);
	Vector3 to = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

	m_globalConstantData.view = XMMatrixLookToLH(eyepos, to, up);
	m_globalConstantData.proj =
		XMMatrixPerspectiveFovLH(XMConvertToRadians(80.0f), 1600.0f / 900.0f, 0.001f, 1000.0f);
	
	m_globalConstantData.view = m_globalConstantData.view.Transpose();
	m_globalConstantData.proj = m_globalConstantData.proj.Transpose();

	m_chunks = new Chunk[CHUNK_SIZE];
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		m_chunks[i] = Chunk(i * Chunk::BLOCK_SIZE, 0, 0);
	}
}

App::~App()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(m_hwnd);

	delete[] m_chunks;
}

bool App::Initialize()
{
	if (!InitWindow())
		return false;

	if (!InitDirectX())
		return false;

	if (!InitGUI())
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
			ImGui::End();
			ImGui::Render(); // 렌더링할 것들 기록 끝

			Update(ImGui::GetIO().DeltaTime);
			Render(); // 우리가 구현한 렌더링

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // GUI 렌더링

			m_swapChain->Present(1, 0);
		}
	}
}

void App::Update(float dt)
{
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		m_chunks[i].Update(m_context, dt);
	}
}

void App::Render()
{
	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_context->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
	m_context->ClearDepthStencilView(
		m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), m_depthStencilView.Get());

	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetConstantBuffers(1, 1, m_globalConstantBuffer.GetAddressOf());
	m_context->VSSetShader(m_vertexShader.Get(), 0, 0);

	m_context->RSSetState(m_rasterizerState.Get());
	m_context->RSSetViewports(1, &m_viewport);

	m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
	// m_context->PSSetSamplers();
	// m_context->PSSetShaderResources();

	for (int i = 0; i < CHUNK_SIZE; ++i) {
		m_chunks[i].Render(m_context);
	}
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
	if (!Utils::CreateDeviceAndSwapChain(
			m_device, m_context, m_swapChain, m_hwnd, m_width, m_height)) {
		std::cout << "failed create device" << std::endl;
		return false;
	}

	if (!Utils::CreateVertexShader(m_device, L"VertexShader.hlsl", m_vertexShader, m_inputLayout)) {
		std::cout << "failed create vs" << std::endl;
		return false;
	}

	if (!Utils::CreatePixelShader(m_device, L"PixelShader.hlsl", m_pixelShader)) {
		std::cout << "failed create ps" << std::endl;
		return false;
	}

	if (!Utils::CreateRasterizerState(m_device, m_rasterizerState)) {
		std::cout << "failed create rasterizer state" << std::endl;
		return false;
	}

	if (!Utils::CreateRenderTargetBuffer(m_device, m_renderTargetBuffer, m_width, m_height)) {
		std::cout << "failed create render target buffer" << std::endl;
		return false;
	}

	if (!Utils::CreateRenderTargetView(m_device, m_renderTargetBuffer, m_renderTargetView)) {
		std::cout << "failed create render target view" << std::endl;
		return false;
	}

	if (!Utils::CreateDepthStencilBuffer(m_device, m_depthStencilBuffer, m_width, m_height)) {
		std::cout << "failed create depth stencil buffer" << std::endl;
		return false;
	}

	if (!Utils::CreateDepthStencilView(m_device, m_depthStencilBuffer, m_depthStencilView)) {
		std::cout << "failed create depth stencil view" << std::endl;
		return false;
	}

	if (!Utils::CreateDepthStencilState(m_device, m_depthStencilState)) {
		std::cout << "failed create depth stencil state" << std::endl;
		return false;
	}

	if (!Utils::CreateConstantBuffer(m_device, m_globalConstantBuffer, m_globalConstantData)) {
		std::cout << "failed create constant buffer" << std::endl;
		return false;
	}

	Utils::UpdateViewport(m_viewport, 0, 0, (FLOAT)m_width, (FLOAT)m_height);

	HRESULT ret = m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.GetAddressOf()));
	if (FAILED(ret)) {
		std::cout << "failed get back buffer" << std::endl;
		return false;
	}
	if (!Utils::CreateRenderTargetView(m_device, m_backBuffer, m_backBufferRTV)) {
		std::cout << "failed create back buffer RTV" << std::endl;
		return false;
	}

	for (int i = 0; i < CHUNK_SIZE; ++i) {
		m_chunks[i].Initialize(m_device);
	}

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
	if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
		return false;
	}

	if (!ImGui_ImplWin32_Init(m_hwnd)) {
		return false;
	}

	return true;
}
