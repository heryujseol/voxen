#include "Camera.h"
#include "DXUtils.h"

Camera::Camera()
	: m_projFovAngleY(80.0f), m_nearZ(0.01f), m_farZ(800.0f), m_aspectRatio(16.0f / 9.0f),
	  m_eyePos(0.0f, 0.0f, 0.0f), m_chunkPos(0.0f, 0.0f, 0.0f), m_to(0.0f, 0.0f, 1.0f),
	  m_up(0.0f, 1.0f, 0.0f), m_right(1.0f, 0.0f, 0.0f), m_viewNdcX(0.0f), m_viewNdcY(0.0f),
	  m_speed(20.0f), m_isOnConstantDirtyFlag(false)
{
	m_constantData.view = Matrix();
	m_constantData.proj = Matrix();
}

Camera::~Camera() {}

bool Camera::Initialize(Vector3 pos)
{
	m_eyePos = pos;
	m_chunkPos = Utils::CalcChunkPos(m_eyePos);

	m_constantData.view = GetViewMatrix();
	m_constantData.proj = GetProjectionMatrix();
	m_constantData.eyePos = m_eyePos;

	CameraConstantData tempConstantData;
	tempConstantData.view = m_constantData.view.Transpose();
	tempConstantData.proj = m_constantData.proj.Transpose();
	tempConstantData.eyePos = m_constantData.eyePos;
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, tempConstantData)) {
		std::cout << "failed create constant buffer" << std::endl;
		return false;
	}

	return true;
}

void Camera::Update(float dt, bool keyPressed[256], float mouseX, float mouseY)
{
	UpdatePosition(keyPressed, dt);
	UpdateViewDirection(mouseX, mouseY);

	if (m_isOnConstantDirtyFlag) {
		CameraConstantData tempConstantData;
		tempConstantData.view = GetViewMatrix().Transpose();
		tempConstantData.proj = GetProjectionMatrix().Transpose();
		DXUtils::UpdateConstantBuffer(m_constantBuffer, tempConstantData);

		m_isOnConstantDirtyFlag = false;
	}
}

void Camera::UpdatePosition(bool keyPressed[256], float dt)
{
	if (keyPressed['W']) {
		MoveForward(dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['S']) {
		MoveForward(-dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['D']) {
		MoveRight(dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['A']) {
		MoveRight(-dt);
		m_isOnConstantDirtyFlag = true;
	}

	Vector3 newChunkPos = Utils::CalcChunkPos(m_eyePos);
	m_diffChunkPos = newChunkPos - m_chunkPos;
	m_chunkPos = newChunkPos;
}

void Camera::UpdateViewDirection(float ndcX, float ndcY)
{
	if (m_viewNdcX == ndcX && m_viewNdcY == ndcY)
		return;

	m_isOnConstantDirtyFlag = true;

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

Vector3 Camera::GetPosition() { return m_eyePos; }

Vector3 Camera::GetChunkPosition() { return m_chunkPos; }

float Camera::GetDistance() { return m_farZ; }

Matrix Camera::GetViewMatrix() { return XMMatrixLookToLH(m_eyePos, m_to, m_up); }

Matrix Camera::GetProjectionMatrix()
{
	return XMMatrixPerspectiveFovLH(
		XMConvertToRadians(m_projFovAngleY), m_aspectRatio, m_nearZ, m_farZ);
}

ComPtr<ID3D11Buffer> Camera::GetConstantBuffer() { return m_constantBuffer; }

Vector3 Camera::GetDiffChunkPos() { return m_diffChunkPos; }

void Camera::MoveForward(float dt) { m_eyePos += m_to * m_speed * dt; }

void Camera::MoveRight(float dt) { m_eyePos += m_right * m_speed * dt; }