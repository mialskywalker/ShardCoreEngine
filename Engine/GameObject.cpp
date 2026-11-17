#include "GameObject.h"
#include "Scene.h"

#include <algorithm>

#include "Component.h"

#include "ComponentFactory.h"
#include "TransformComponent.h"

GameObject::GameObject(Scene* scene, uint64_t uid, const std::string& name)
	: m_Scene(scene), m_UID(uid), m_Name(name)
{
	AddComponent(ComponentType::Transform);
}

GameObject::~GameObject() {}

bool GameObject::Init()
{
	if (m_Initialized)
	{
		return true;
	}

	for (auto& component : m_Components)
	{
		if (!component->Init())
		{
			return false;
		}
	}

	m_Initialized = true;

	return true;
}

void GameObject::Update(float deltaTime)
{
	for (auto& component : m_Components)
	{
		if (component->IsEnabled())
		{
			component->Update(deltaTime);
		}
	}
}

void GameObject::SetName(const std::string& name)
{
	m_Name = name;
}

void GameObject::SetActive(bool active)
{
	m_Active = active;
}

bool GameObject::SetParent(GameObject* newParent)
{
	if (newParent && newParent->m_Scene != m_Scene)
	{
		return false;
	}

	if (newParent == this)
	{
		return false;
	}

	if (newParent == m_Parent)
	{
		return true;
	}

	// Circular check
	GameObject* current = newParent;

	while (current)
	{
		if (current == this)
		{
			return false;
		}

		current = current->m_Parent;
	}

	// Remove from the previous parent's children
	if (m_Parent)
	{
		auto& children = m_Parent->m_Children;
		auto it = std::find(children.begin(), children.end(), this);

		if (it != children.end())
		{
			children.erase(it);
		}
	}

	// Assign new parent
	m_Parent = newParent;

	if (m_Parent)
	{
		m_Parent->m_Children.push_back(this);
	}

	return true;
}

Component* GameObject::AddComponent(ComponentType type)
{
	if (type == ComponentType::Transform && GetTransform())
	{
		return nullptr;
	}

	std::unique_ptr<Component> component = ComponentFactory::Create(type, this);
	if (!component)
	{
		return nullptr;
	}

	if (m_Initialized)
	{
		if (!component->Init())
		{
			return nullptr;
		}
	}

	Component* rawPtr = component.get();
	m_Components.push_back(std::move(component));
	
	return rawPtr;
}

Component* GameObject::GetComponent(ComponentType type) const
{
	for (const auto& comp : m_Components)
	{
		if (comp->GetType() == type)
		{
			return comp.get();
		}
	}

	return nullptr;
}

TransformComponent* GameObject::GetTransform() const
{
	return static_cast<TransformComponent*>(GetComponent(ComponentType::Transform));
}
