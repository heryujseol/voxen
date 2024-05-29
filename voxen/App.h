#pragma once

#include <windows.h>

#include "ChunkManager.h"
#include "Camera.h"
#include "Skybox.h"
#include "Cloud.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class App {
public:
	App();
	~App();

	LRESULT EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Initialize();
	void Run();


private:
	bool InitWindow();
	bool InitDirectX();
	bool InitGUI();
	bool InitScene();

	void Update(float dt);
	void Render();

	UINT m_width;
	UINT m_height;
	HWND m_hwnd;

	ChunkManager m_chunkManager;
	Camera m_camera;
	Skybox m_skybox;
	Cloud m_cloud;

	bool m_keyPressed[256];
	bool m_keyToggle[256];
	float m_mouseNdcX;
	float m_mouseNdcY;
};
