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

	Vector3 GetPosition();
	Vector3 GetChunkPosition();

	float GetDistance();
	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();

	void SetPosition(Vector3 newPos);

	void MoveForward(float dt);
	void MoveRight(float dt);

	void UpdatePosition(bool keyPressed[256], float dt);
	void UpdateViewDirection(float mouseX, float mouseY);

	bool IsOnConstantDirtyFlag();
	void OffConstantDirtyFlag();

	bool IsOnChunkDirtyFlag();
	void OffChunkDirtyFlag();

private:
	float m_projFovAngleY;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	Vector3 m_eyePos;
	Vector3 m_chunkPos;
	Vector3 m_to;
	Vector3 m_up;
	Vector3 m_right;


	float m_viewNdcX;
	float m_viewNdcY;

	float m_speed;

	bool m_constantDirtyFlag;
	bool m_chunkDirtyFlag;
};