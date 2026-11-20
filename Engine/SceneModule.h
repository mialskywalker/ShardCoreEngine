#pragma once

#include <memory>
#include <string>

class Scene;

class SceneModule
{
public:
	SceneModule();
	~SceneModule();

	Scene* CreateScene(const std::string& name);
	Scene* GetActiveScene() const;

	void Update(float deltaTime);
	void RequestSceneChange(const std::string& name);

private:
	std::unique_ptr<Scene> m_ActiveScene;
	bool m_HasPendingScene = false;
	std::string m_PendingSceneName;
};