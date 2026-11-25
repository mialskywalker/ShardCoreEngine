#include "SceneModule.h"
#include "Scene.h"
#include "Logger.h"

SceneModule::SceneModule() {}

SceneModule::~SceneModule() {}

Scene* SceneModule::CreateScene(const std::string& name)
{
	m_ActiveScene = std::make_unique<Scene>(name);
	return GetActiveScene();
}

Scene* SceneModule::GetActiveScene() const
{
	return m_ActiveScene.get();
}

void SceneModule::Update(float deltaTime)
{
	Scene* current = GetActiveScene();
	if (current)
	{
		current->Update(deltaTime);
	}

	if (m_PendingScene)
	{
		m_ActiveScene = std::move(m_PendingScene);
	}
}

void SceneModule::RequestSceneChange(const std::string& name)
{
	std::unique_ptr<Scene> newScene = std::make_unique<Scene>(name);
	if (!newScene->Init())
	{
		Logger::Error("Request Scene Change FAILED!", LogType::Engine);
		return;
	}

	m_PendingScene = std::move(newScene);
}

bool SceneModule::RequestSceneChange(std::unique_ptr<Scene> scene)
{
	if (!scene)
	{
		return false;
	}

	m_PendingScene = std::move(scene);
	return true;
}
