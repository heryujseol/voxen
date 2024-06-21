#pragma once

#include <windows.h>

#include "ChunkManager.h"
#include "Camera.h"
#include "Skybox.h"
#include "Cloud.h"
#include "PostEffect.h"

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

	static const UINT WIDTH = 1920;
	static const UINT HEIGHT = 1080;
	static const UINT MIRROR_WIDTH = WIDTH / 2;
	static const UINT MIRROR_HEIGHT = HEIGHT / 2;
	static const UINT ENV_MAP_SIZE = WIDTH / 8;

private:
	bool InitWindow();
	bool InitDirectX();
	bool InitGUI();
	bool InitScene();

	void Update(float dt);
	void Render();

	void RenderDepthOnly();
	void RenderEnvMap();
	void RenderBasic();
	void RenderMirror();

	HWND m_hwnd;

	ChunkManager m_chunkManager;
	Camera m_camera;
	Skybox m_skybox;
	Cloud m_cloud;
	PostEffect m_postEffect;

	bool m_keyPressed[256];
	bool m_keyToggle[256];
	float m_mouseNdcX;
	float m_mouseNdcY;
};
