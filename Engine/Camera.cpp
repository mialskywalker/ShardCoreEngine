#include "Camera.h"
#include "Logger.h"

Camera::Camera() {}

Camera::~Camera() {}

bool Camera::Init()
{
	m_View = Matrix::CreateLookAt(m_Position, m_Target, Vector3::Up);

	if (m_AspectRatio <= 0.0f)
		SetAspectRatio(float(1280) / float(720));
	SetFOV(DirectX::XM_PIDIV4);

	m_Projection = Matrix::CreatePerspectiveFieldOfView(m_FOV, m_AspectRatio, 0.1f, 1000.0f);

	Logger::Log("Camera initialized");

	return true;
}

void Camera::SetAspectRatio(float newAspectRatio)
{
	m_AspectRatio = newAspectRatio;
}

void Camera::SetFOV(float newFOV)
{
	m_FOV = newFOV;
}
