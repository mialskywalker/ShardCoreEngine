#include "Camera.h"
#include "Logger.h"
#include "Input.h"
#include "Mouse.h"

Camera::Camera() {}

Camera::~Camera() {}

bool Camera::Init()
{
	m_CurrentPosition = m_StartingPosition;
	m_View = Matrix::CreateLookAt(m_CurrentPosition, m_Target, Vector3::Up);

	if (m_AspectRatio <= 0.0f)
		SetAspectRatio(float(1280) / float(720));
	SetFOV(DirectX::XM_PIDIV4);

	m_Projection = Matrix::CreatePerspectiveFieldOfView(m_FOV, m_AspectRatio, 0.1f, 1000.0f);

	Logger::Log("Camera initialized");

	return true;
}

void Camera::Update(float deltaTime)
{
	Vector2 rotate = Vector2::Zero;

	const DirectX::Mouse::State& mouseState = Input::mouse.get()->GetState();

	int mouseScrollWheel = mouseState.scrollWheelValue;

	if (mouseState.rightButton)
	{
		rotate.x = float(mousePosX - mouseState.x) * 0.15f;
		rotate.y = float(mousePosY - mouseState.y) * 0.15f;
	}

	mousePosX = mouseState.x;
	mousePosY = mouseState.y;

	yaw += DirectX::XMConvertToRadians(rotate.x);
	pitch += DirectX::XMConvertToRadians(rotate.y);

	m_CurrentRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0.0f);

	Quaternion inverse;
	m_CurrentRotation.Inverse(inverse);

	m_View = Matrix::CreateFromQuaternion(inverse);
	m_View.Translation(Vector3::Transform(-m_CurrentPosition, inverse));

	m_Projection = Matrix::CreatePerspectiveFieldOfView(m_FOV, m_AspectRatio, 0.1f, 1000.0f);

}

void Camera::SetAspectRatio(float newAspectRatio)
{
	m_AspectRatio = newAspectRatio;
}

void Camera::SetFOV(float newFOV)
{
	m_FOV = newFOV;
}

void Camera::SetTarget(const Vector3 target)
{
	m_Target = target;
}
