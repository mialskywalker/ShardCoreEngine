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

	if (m_HasPendingScene)
	{
		auto newScene = std::make_unique<Scene>(m_PendingSceneName);
		if (newScene->Init())
		{
			m_ActiveScene = std::move(newScene);
		}
		else
		{
			Logger::Error(m_PendingSceneName + " scene could not be created!", LogType::Engine);
		}

		m_HasPendingScene = false;
	}
}

void SceneModule::RequestSceneChange(const std::string& name)
{
	m_PendingSceneName = name;
	m_HasPendingScene = true;
}
