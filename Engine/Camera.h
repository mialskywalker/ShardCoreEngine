#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	bool Init();

	void SetAspectRatio(float newAspectRatio);
	void SetFOV(float newFOV);

private:
	Vector3						m_Position = Vector3(0.0f, 5.0f, 5.0f);
	Vector3						m_Target = Vector3::Zero;
	Matrix						m_View;
	Matrix						m_Projection;

	float						m_AspectRatio = 0.0f;
	float						m_FOV = 0.0f;

public:
	const Vector3& GetPosition() { return m_Position; }
	const Vector3& GetTarget() { return m_Target; }
	const Matrix& GetView() const { return m_View; }
	const Matrix& GetProjection() const { return m_Projection; }

};