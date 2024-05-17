#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "directxtk/SimpleMath.h"

#include "Chunk.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera {
public:
	Camera();
	~Camera();

	bool Initialize(Vector3 pos);

	void Update(float dt, bool keyPressed[256], float mouseX, float mouseY);

	Vector3 GetPosition();
	Vector3 GetChunkPosition();
	float GetDistance();
	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();
	ComPtr<ID3D11Buffer> GetConstantBuffer();

	bool m_isOnConstantDirtyFlag;
	bool m_isOnChunkDirtyFlag;

private:
	void UpdatePosition(bool keyPressed[256], float dt);
	void UpdateViewDirection(float mouseX, float mouseY);

	void MoveForward(float dt);
	void MoveRight(float dt);

	float m_projFovAngleY;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	Vector3 m_eyePos;
	Vector3 m_chunkPos;
	Vector3 m_forward;
	Vector3 m_up;
	Vector3 m_right;

	float m_viewNdcX;
	float m_viewNdcY;

	float m_speed;


	CameraConstantData m_constantData;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};