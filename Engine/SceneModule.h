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
	bool RequestSceneChange(std::unique_ptr<Scene> scene);

private:
	std::unique_ptr<Scene> m_ActiveScene;
	std::unique_ptr<Scene> m_PendingScene;
};