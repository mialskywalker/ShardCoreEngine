#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	bool Init();
	void Update(float deltaTime);

	void SetAspectRatio(float newAspectRatio);
	void SetFOV(float newFOV);
	void SetTarget(const Vector3 target);

private:
	const float MOVE_SPEED = 2.0f;

	Matrix						m_View;
	Matrix						m_Projection;

	Vector3						m_StartingPosition = Vector3(0.0f, 5.0f, 5.0f);
	Vector3						m_CurrentPosition = Vector3::Zero;
	Vector3						m_Target = Vector3::Zero;

	Quaternion					m_StartingRotation;
	Quaternion					m_CurrentRotation;

	float						m_AspectRatio = 0.0f;
	float						m_FOV = 0.0f;

	float						yaw = 0.0f;
	float						pitch = 0.0f;

	float						mousePosX = 0.0f;
	float						mousePosY = 0.0f;

	int							speed = 1;
	int							prevWheel = 0;

public:
	const Matrix GetView() { return m_View; }
	const Matrix GetProjection() { return m_Projection; }

	const Vector3 GetPosition() { return m_CurrentPosition; }
	const Vector3 GetTarget() { return m_Target; }

	const float getMoveSpeed() { return MOVE_SPEED; }

};