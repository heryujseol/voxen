#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "directxtk/SimpleMath.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;

class Camera {
public:
	Camera();
	~Camera();


	Vector3 GetPosition();
	float GetDistance();
	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();

	void SetPosition(Vector3 newPos);

	void MoveForward(float dt);
	void MoveRight(float dt);

	void UpdatePosition(bool keyPressed[256], float dt);
	void UpdateViewDirection(float mouseX, float mouseY);

	bool IsDirtyFlag();
	void OffDirtyFlag();

private:
	float m_projFovAngleY;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	Vector3 m_eyePos;
	Vector3 m_to;
	Vector3 m_up;
	Vector3 m_right;

	float m_viewNdcX;
	float m_viewNdcY;

	float m_speed;

	bool m_dirtyFlag;
};