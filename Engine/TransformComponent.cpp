#include "TransformComponent.h"
#include "GameObject.h"

TransformComponent::TransformComponent(GameObject* owner)
	: Component(owner, ComponentType::Transform)
{
}

Matrix TransformComponent::GetLocalMatrix() const
{
	const Matrix scale = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z);
	const Matrix rotation = Matrix::CreateFromQuaternion(m_Rotation);
	const Matrix translation = Matrix::CreateTranslation(m_Position.x, m_Position.y, m_Position.z);

	return scale * rotation * translation;
}

Matrix TransformComponent::GetWorldMatrix() const
{
	const Matrix local = GetLocalMatrix();
	GameObject* parent = GetOwner()->GetParent();

	if (parent)
	{
		return local * parent->GetTransform()->GetWorldMatrix();
	}

	return local;
}
