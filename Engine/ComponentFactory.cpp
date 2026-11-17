#include "ComponentFactory.h"
#include "Component.h"

#include "TransformComponent.h"

std::unique_ptr<Component> ComponentFactory::Create(ComponentType type, GameObject* owner)
{
	switch (type)
	{
	case ComponentType::Transform:
		return std::make_unique<TransformComponent>(owner);

	default:
		return nullptr;
	}
}