#include "ComponentFactory.h"
#include "Component.h"

#include "TransformComponent.h"
#include "MeshComponent.h"

std::unique_ptr<Component> ComponentFactory::Create(ComponentType type, GameObject* owner)
{
	switch (type)
	{
	case ComponentType::Transform:
		return std::make_unique<TransformComponent>(owner);

	case ComponentType::Mesh:
		return std::make_unique<MeshComponent>(owner);

	default:
		return nullptr;
	}
}