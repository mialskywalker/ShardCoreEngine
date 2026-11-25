#pragma once

#include <cstdint>
#include <string>

class Scene;
class GameObject;

class WindowHierarchy
{
public:
	void Draw(Scene* scene);

	void ClearSelection();

	uint64_t GetSelectedUID() const { return m_SelectedUID; }

private:
	void DrawGameObject(GameObject* go);

private:
	uint64_t m_SelectedUID = 0;
	Scene* m_LastScene = nullptr;
	uint64_t m_PendingCreateChildUID = 0;
	uint64_t m_PendingDeleteUID = 0;
	std::string m_SceneName;
};