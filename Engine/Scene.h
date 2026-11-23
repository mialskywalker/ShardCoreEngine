#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <string>

class GameObject;

class Scene
{
public:
	Scene(const std::string& name);
	~Scene();

	bool Init();
	void Update(float deltaTime);

public:
	GameObject* CreateGameObject(const std::string& name);
	GameObject* CreateGameObjectWithUID(const std::string& name, uint64_t uid);
	GameObject* GetGameObject(uint64_t uid);
	bool RemoveGameObject(uint64_t uid);
	const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_GameObjects; }
	const std::string& GetSceneName() const { return m_Name; }

private:
	std::string m_Name;
	uint64_t m_NextUID = 1;
	std::vector<std::unique_ptr<GameObject>> m_GameObjects;
	bool m_IsUpdating = false;
	std::vector<uint64_t> m_PendingRemovals;
	std::vector<std::unique_ptr<GameObject>> m_PendingAdditions;
};