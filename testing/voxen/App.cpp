#include "App.h"

App* g_app = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return g_app->EventHandler(hwnd, uMsg, wParam, lParam);
}

App::App()
	: m_width(1920), m_height(1080), m_hwnd(), m_viewport(), m_map(), m_camera(), m_mouseNdcX(0.0f),
	  m_mouseNdcY(0.0f)
{
	g_app = this;

	m_loadFuture = std::async(std::launch::async, &App::LoadChunks, this);
}

App::~App()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(m_hwnd);

	m_map.clear();
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

	InitMesh();

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
	m_camera.UpdatePosition(m_keyPressed, dt);
	m_camera.UpdateViewDirection(m_mouseNdcX, m_mouseNdcY);

	if (m_camera.IsOnConstantDirtyFlag()) {
		m_globalConstantData.view = m_camera.GetViewMatrix();
		m_globalConstantData.proj = m_camera.GetProjectionMatrix();

		GlobalConstantData tempConstantData;
		tempConstantData.view = m_globalConstantData.view.Transpose();
		tempConstantData.proj = m_globalConstantData.proj.Transpose();
		Utils::UpdateConstantBuffer(m_context, m_globalConstantBuffer, tempConstantData);

		m_camera.OffConstantDirtyFlag();
	}

	if (m_camera.IsOnChunkDirtyFlag()) {
		UpdateChunkList();
		m_camera.OffChunkDirtyFlag();
		if (!m_unloadChunkList.empty()) {
			UnloadChunks();
		}
	}
	
	if (!m_loadChunkList.empty()) {
		if (m_loadFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			m_loadFuture = std::async(std::launch::async, &App::LoadChunks, this);
		}
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
	m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	std::vector<ID3D11ShaderResourceView*> pptr = { m_textureSRV1.Get(), m_textureSRV2.Get(),
		m_textureSRV3.Get(), m_textureSRV4.Get() };
	m_context->PSSetShaderResources(0, 4, pptr.data());

	for (auto& p : m_map) {
		if (p.second.IsEmpty() || !p.second.IsLoaded())
			continue;

		p.second.Render(m_context);
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

	m_globalConstantData.view = m_camera.GetViewMatrix();
	m_globalConstantData.proj = m_camera.GetProjectionMatrix();
	GlobalConstantData tempConstantData;
	tempConstantData.view = m_globalConstantData.view.Transpose();
	tempConstantData.proj = m_globalConstantData.proj.Transpose();
	if (!Utils::CreateConstantBuffer(m_device, m_globalConstantBuffer, tempConstantData)) {
		std::cout << "failed create constant buffer" << std::endl;
		return false;
	}

	HRESULT ret = m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.GetAddressOf()));
	if (FAILED(ret)) {
		std::cout << "failed get back buffer" << std::endl;
		return false;
	}
	if (!Utils::CreateRenderTargetView(m_device, m_backBuffer, m_backBufferRTV)) {
		std::cout << "failed create back buffer RTV" << std::endl;
		return false;
	}

	Utils::UpdateViewport(m_viewport, 0, 0, (FLOAT)m_width, (FLOAT)m_height);

	if (!Utils::CreateSamplerState(m_device, m_samplerState)) {
		std::cout << "failed create sampler state" << std::endl;
		return false;
	}

	if (!Utils::CreateTexture2DFromFile(
			m_device, m_texture, "../assets/grass_block_side_overlay.png")) {
		std::cout << "failed create texture" << std::endl;
		return false;
	}
	if (!Utils::CreateShaderResourceView(m_device, m_texture, m_textureSRV1)) {
		std::cout << "failed create shader resource view" << std::endl;
		return false;
	}

	if (!Utils::CreateTexture2DFromFile(m_device, m_texture, "../assets/grass_block_top.png")) {
		std::cout << "failed create texture" << std::endl;
		return false;
	}
	if (!Utils::CreateShaderResourceView(m_device, m_texture, m_textureSRV2)) {
		std::cout << "failed create shader resource view" << std::endl;
		return false;
	}

	if (!Utils::CreateTexture2DFromFile(m_device, m_texture, "../assets/dirt.png")) {
		std::cout << "failed create texture" << std::endl;
		return false;
	}
	if (!Utils::CreateShaderResourceView(m_device, m_texture, m_textureSRV3)) {
		std::cout << "failed create shader resource view" << std::endl;
		return false;
	}

	if (!Utils::CreateTexture2DFromFile(m_device, m_texture, "../assets/grass_color_map.png")) {
		std::cout << "failed create texture" << std::endl;
		return false;
	}
	if (!Utils::CreateShaderResourceView(m_device, m_texture, m_textureSRV4)) {
		std::cout << "failed create shader resource view" << std::endl;
		return false;
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

void App::InitMesh()
{
	Vector3 cameraOffset = m_camera.GetChunkPosition();

	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int x = (int)cameraOffset.x + Chunk::BLOCK_SIZE * (i - CHUNK_SIZE / 2);
				int y = (int)cameraOffset.y + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraOffset.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				m_map[std::make_tuple(x, y, z)] = Chunk(x, y, z);
				m_map[std::make_tuple(x, y, z)].Initialize(m_device);
			}
		}
	}
}

void App::UpdateChunkList()
{
	Vector3 cameraOffset = m_camera.GetChunkPosition();

	std::map<std::tuple<int, int, int>, bool> loadedChunkMap;
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				int x = (int)cameraOffset.x + Chunk::BLOCK_SIZE * (i - CHUNK_SIZE / 2);
				int y = (int)cameraOffset.y + Chunk::BLOCK_SIZE * (j - CHUNK_SIZE / 2);
				int z = (int)cameraOffset.z + Chunk::BLOCK_SIZE * (k - CHUNK_SIZE / 2);

				if (m_map.find(std::make_tuple(x, y, z)) == m_map.end())
					m_loadChunkList.push_back(Vector3((float)x, (float)y, (float)z)); // will be loaded
				else
					loadedChunkMap[std::make_tuple(x, y, z)] = true;
			}
		}
	}

	for (auto& p : m_map) { // {1 , 2, 3} -> {1, 2}
		if (loadedChunkMap.find(p.first) == loadedChunkMap.end() && m_map[p.first].IsLoaded()) {
			int x = std::get<0>(p.first);
			int y = std::get<1>(p.first);
			int z = std::get<2>(p.first);

			m_unloadChunkList.push_back(Vector3((float)x, (float)y, (float)z));
		}
	}
}

void App::LoadChunks()
{
	int count = 0;
	while (!m_loadChunkList.empty()) {
		Vector3 pos = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;
		m_map[std::make_tuple(x, y, z)] = Chunk(x, y, z);
		m_map[std::make_tuple(x, y, z)].Initialize(m_device);
		count++;

		if (count == 3) //  chunks loading per each frame
			return;
	}
}

void App::UnloadChunks()
{
	for (int i = 0; i < m_unloadChunkList.size(); ++i) 
	{
		Vector3 pos = m_unloadChunkList[i];

		// Chunk를 지워야하잖아
		// 메모리 초기화
		m_map.erase(std::make_tuple((int)pos.x, (int)pos.y, (int)pos.z));
	}
	m_unloadChunkList.clear();
}