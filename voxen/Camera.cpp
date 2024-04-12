#include "Camera.h"


Camera::Camera()
	: m_projFovAngleY(80.0f), m_nearZ(0.01f), m_farZ(360.0f), m_aspectRatio(16.0f / 9.0f),
	  m_eyePos(16.0f, 50.0f, 16.0f), m_chunkPos(0.0f, 0.0f, 0.0f),
	  m_to(0.0f, 0.0f, 1.0f),
	  m_up(0.0f, 1.0f, 0.0f), m_right(1.0f, 0.0f, 0.0f), m_viewNdcX(0.0f), m_viewNdcY(0.0f),
	  m_speed(20.0f), m_constantDirtyFlag(false), m_chunkDirtyFlag(false)
{
	m_chunkPos = Utils::CalcChunkOffset(m_eyePos);
}

Camera::~Camera() {}

Vector3 Camera::GetPosition() { return m_eyePos; }

Vector3 Camera::GetChunkPosition() { return m_chunkPos; }

float Camera::GetDistance() { return m_farZ; }

Matrix Camera::GetViewMatrix() { return XMMatrixLookToLH(m_eyePos, m_to, m_up); }

Matrix Camera::GetProjectionMatrix()
{
	return XMMatrixPerspectiveFovLH(
		XMConvertToRadians(m_projFovAngleY), m_aspectRatio, m_nearZ, m_farZ);
}

void Camera::SetPosition(Vector3 newPos) { m_eyePos = newPos; }

void Camera::MoveForward(float dt) { m_eyePos += m_to * m_speed * dt; }

void Camera::MoveRight(float dt) { m_eyePos += m_right * m_speed * dt; }

void Camera::UpdatePosition(bool keyPressed[256], float dt)
{
	if (keyPressed['W']) {
		MoveForward(dt);
		m_constantDirtyFlag = true;
	}
	if (keyPressed['S']) {
		MoveForward(-dt);
		m_constantDirtyFlag = true;
	}
	if (keyPressed['D']) {
		MoveRight(dt);
		m_constantDirtyFlag = true;
	}
	if (keyPressed['A']) {
		MoveRight(-dt);
		m_constantDirtyFlag = true;
	}

	if (m_constantDirtyFlag) {
		Vector3 newChunkOffset = Utils::CalcChunkOffset(m_eyePos);
		if (newChunkOffset != m_chunkPos) {
			m_chunkPos = newChunkOffset;
			m_chunkDirtyFlag = true;
		}
	}
}

void Camera::UpdateViewDirection(float ndcX, float ndcY)
{
	if (m_viewNdcX == ndcX && m_viewNdcY == ndcY)
		return;

	m_constantDirtyFlag = true;

	m_viewNdcX = ndcX;
	m_viewNdcY = ndcY;

	float thetaHorizontal = DirectX::XM_PI * m_viewNdcX;
	float thetaVertical = -DirectX::XM_PIDIV2 * m_viewNdcY;

	// using Quaternion not Euler
	Vector3 basisX = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 basisY = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 basisZ = Vector3(0.0f, 0.0f, 1.0f);

	Quaternion q = Quaternion(basisY * sinf(thetaHorizontal * 0.5f), cosf(thetaHorizontal * 0.5f));
	m_to = Vector3::Transform(basisZ, Matrix::CreateFromQuaternion(q));
	m_right = Vector3::Transform(basisX, Matrix::CreateFromQuaternion(q));

	q = Quaternion(m_right * sinf(thetaVertical * 0.5f), cosf(thetaVertical * 0.5f));
	m_to = Vector3::Transform(m_to, Matrix::CreateFromQuaternion(q));
	m_up = Vector3::Transform(basisY, Matrix::CreateFromQuaternion(q));
}

bool Camera::IsOnConstantDirtyFlag() { return m_constantDirtyFlag; }

void Camera::OffConstantDirtyFlag() { m_constantDirtyFlag = false; }

bool Camera::IsOnChunkDirtyFlag() { return m_chunkDirtyFlag; }

void Camera::OffChunkDirtyFlag() { m_chunkDirtyFlag = false; }