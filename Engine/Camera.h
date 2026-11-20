#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	bool Init(uint32_t width, uint32_t height);

	void SetAspectRatio(float newAspectRatio);
	void SetFOV(float newFOV);
	void SetTarget(const Vector3 target);

private:
	Matrix						m_View;
	Matrix						m_Projection;

	Vector3						m_StartingPosition = Vector3(0.0f, 1.0f, 10.0f);
	Vector3						m_CurrentPosition = Vector3::Zero;
	Vector3						m_Target = Vector3::Zero;

	Quaternion					m_StartingRotation = Quaternion::Identity;
	Quaternion					m_CurrentRotation;

	float						m_AspectRatio = 0.0f;
	float						m_FOV = 0.0f;
	float						m_NearPlane = 0.1f;
	float						m_FarPlane = 1000.0f;

public:
	const Matrix GetView() { return m_View; }
	const Matrix GetProjection() { return m_Projection; }

	const Vector3 GetPosition() { return m_CurrentPosition; }
	const Vector3 GetTarget() { return m_Target; }

	Quaternion GetRotation() const { return m_CurrentRotation; }

	void SetPosition(const Vector3& position);
	void SetRotation(const Quaternion& rotation);

private:
	void UpdateProjection();
	void UpdateView();
};