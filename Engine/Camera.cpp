#include "Camera.h"
#include "Logger.h"

Camera::Camera() {}

Camera::~Camera() {}

bool Camera::Init(uint32_t width, uint32_t height)
{
	m_CurrentPosition = m_StartingPosition;
	m_CurrentRotation = m_StartingRotation;
	m_NearPlane = 0.1f;
	m_FarPlane = 1000.f;

	SetFOV(DirectX::XM_PIDIV4);
	SetAspectRatio(float(width) / float(height));
	
	UpdateView();

	Logger::Log("Camera initialized", LogType::Engine);

	return true;
}

void Camera::SetAspectRatio(float newAspectRatio)
{
	if (newAspectRatio <= 0.0f)
		return;

	if (std::abs(m_AspectRatio - newAspectRatio) < 0.0001f)
		return;

	m_AspectRatio = newAspectRatio;
	UpdateProjection();
}

void Camera::SetFOV(float newFOV)
{
	m_FOV = newFOV;
}

void Camera::SetTarget(const Vector3 target)
{
	m_Target = target;
}

void Camera::SetPosition(const Vector3& position)
{
	m_CurrentPosition = position;
	UpdateView();
}

void Camera::SetRotation(const Quaternion& rotation)
{
	m_CurrentRotation = rotation;
	UpdateView();
}

void Camera::UpdateProjection()
{
	m_Projection = Matrix::CreatePerspectiveFieldOfView(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::UpdateView()
{
	Quaternion inverse;
	m_CurrentRotation.Inverse(inverse);

	m_View = Matrix::CreateFromQuaternion(inverse);
	m_View.Translation(Vector3::Transform(-m_CurrentPosition, inverse));
}