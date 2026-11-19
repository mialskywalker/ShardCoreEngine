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
	GameObject* GetGameObject(uint64_t uid);
	bool RemoveGameObject(uint64_t uid);

private:
	std::string m_Name;
	uint64_t m_NextUID = 1;
	std::vector<std::unique_ptr<GameObject>> m_GameObjects;
	bool m_IsUpdating = false;
	std::vector<uint64_t> m_PendingRemovals;
};