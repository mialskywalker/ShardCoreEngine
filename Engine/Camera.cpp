#include "Camera.h"
#include "Logger.h"
#include "Input.h"
#include "Mouse.h"
#include "Keyboard.h"

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

	int mouseScrollWheel = Input::GetMouseWheelValue();

	if (Input::IsMouseButtonDown(MouseButton::Right))
	{
		rotate.x = float(mousePosX - Input::GetMouseDeltaX()) * 0.15f;
		rotate.y = float(mousePosY - Input::GetMouseDeltaY()) * 0.15f;

		if (Input::IsKeyDown(KeyCode::LeftShift))
			speed = 3;
		else
			speed = 1;

		if (Input::IsKeyDown(KeyCode::W)) m_CurrentPosition -= Vector3::Transform(Vector3(0, 0, 1), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
		if (Input::IsKeyDown(KeyCode::S)) m_CurrentPosition += Vector3::Transform(Vector3(0, 0, 1), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
		if (Input::IsKeyDown(KeyCode::A)) m_CurrentPosition -= Vector3::Transform(Vector3(1, 0, 0), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
		if (Input::IsKeyDown(KeyCode::D)) m_CurrentPosition += Vector3::Transform(Vector3(1, 0, 0), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
		if (Input::IsKeyDown(KeyCode::Q)) m_CurrentPosition -= Vector3::Transform(Vector3(0, 1, 0), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
		if (Input::IsKeyDown(KeyCode::E)) m_CurrentPosition += Vector3::Transform(Vector3(0, 1, 0), m_CurrentRotation) * MOVE_SPEED * deltaTime * speed;
	}

	if (Input::IsKeyDown(KeyCode::F))
	{
		m_CurrentPosition = m_StartingPosition;
		m_CurrentRotation = m_StartingRotation;
		yaw = 0.0f;
		pitch = 0.0f;
		return;
	}

	if (mouseScrollWheel > prevWheel) m_CurrentPosition -= Vector3::Transform(Vector3(0, 0, 1), m_CurrentRotation);

	if (mouseScrollWheel < prevWheel) m_CurrentPosition += Vector3::Transform(Vector3(0, 0, 1), m_CurrentRotation);

	prevWheel = mouseScrollWheel;

	mousePosX = Input::GetMouseDeltaX();
	mousePosY = Input::GetMouseDeltaY();

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
