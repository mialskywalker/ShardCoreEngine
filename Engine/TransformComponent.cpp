#include "TransformComponent.h"

TransformComponent::TransformComponent(GameObject* owner)
	: Component(owner, ComponentType::Transform)
{
}