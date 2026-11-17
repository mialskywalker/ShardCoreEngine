#pragma once

#include "Component.h"
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Quaternion;

class TransformComponent : public Component
{
public:
	TransformComponent(GameObject* owner);

public:
	const Vector3& GetPosition() const { return m_Position; }
	const Quaternion& GetRotation() const { return m_Rotation; }
	const Vector3& GetScale() const { return m_Scale; }

	void SetPosition(const Vector3& newPosition) { m_Position = newPosition; }
	void SetRotation(const Quaternion& newRotation) { m_Rotation = newRotation; }
	void SetScale(const Vector3& newScale) { m_Scale = newScale; }

private:
	Vector3 m_Position = Vector3::Zero;
	Quaternion m_Rotation = Quaternion::Identity;
	Vector3 m_Scale = Vector3::One;
};