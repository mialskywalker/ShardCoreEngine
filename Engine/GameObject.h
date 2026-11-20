#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include "Component.h"

class Scene;
class TransformComponent;

class GameObject
{
public:
	GameObject(Scene* scene, uint64_t uid, const std::string& name);
	~GameObject();

	bool Init();
	void Update(float deltaTime);

public:
	// Getters
	uint64_t GetUID() const { return m_UID; }
	const std::string& GetName() const { return m_Name; }
	bool IsActive() const { return m_Active; }
	GameObject* GetParent() const { return m_Parent; }
	const std::vector<GameObject*>& GetChildren() const { return m_Children; }
	const std::vector<std::unique_ptr<Component>>& GetAllComponents() const { return m_Components; }

	// Setters
	void SetName(const std::string& name);
	void SetActive(bool active);
	bool SetParent(GameObject* newParent);

	// Components
	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type) const;
	bool RemoveComponent(ComponentType type);

	TransformComponent* GetTransform() const;

private:
	Scene* m_Scene = nullptr;
	uint64_t m_UID;
	std::string m_Name;
	bool m_Active = true;
	bool m_Initialized = false;
	GameObject* m_Parent = nullptr;
	std::vector<GameObject*> m_Children;
	std::vector<std::unique_ptr<Component>> m_Components;
};