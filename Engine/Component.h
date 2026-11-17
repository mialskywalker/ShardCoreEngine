#pragma once

#include "ComponentType.h"

class GameObject;

class Component
{
public:
	explicit Component(GameObject* owner, ComponentType type);
	virtual ~Component() = default;

	virtual bool Init();
	virtual void Update(float deltaTime);

public:
	GameObject* GetOwner() const { return m_Owner; }
	bool IsEnabled() const { return m_Enabled; }
	void SetEnabled(bool enabled) { m_Enabled = enabled; }

	ComponentType GetType() const { return m_Type; }

private:
	GameObject* m_Owner = nullptr;
	const ComponentType m_Type;
	bool m_Enabled = true;
};