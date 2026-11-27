#pragma once

#include <cstdint>
#include <string>

class EditorSelection;
class Scene;
class GameObject;

class WindowHierarchy
{
public:
	explicit WindowHierarchy(EditorSelection& selection) : m_Selection(selection) {}
	void Draw(Scene* scene);

private:
	void DrawGameObject(GameObject* go);

private:
	EditorSelection& m_Selection;
	uint64_t m_PendingCreateChildUID = 0;
	uint64_t m_PendingDeleteUID = 0;
	std::string m_SceneName;
};