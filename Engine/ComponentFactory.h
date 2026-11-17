#pragma once

#include <memory>
#include "ComponentType.h"

class Component;
class GameObject;

class ComponentFactory
{
public:
	static std::unique_ptr<Component> Create(ComponentType type, GameObject* owner);
};