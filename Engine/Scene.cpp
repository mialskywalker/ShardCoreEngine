#include "Scene.h"
#include "GameObject.h"
#include <algorithm>

Scene::Scene(const std::string& name) 
{
	m_Name = name;
}

Scene::~Scene() {}

bool Scene::Init()
{
	return true;
}

void Scene::Update(float deltaTime)
{
	for (auto&& go : m_GameObjects)
	{
		if (go->IsActive())
		{
			go->Update(deltaTime);
		}
	}
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
	std::unique_ptr<GameObject> newGameObject = std::make_unique<GameObject>(this, m_NextUID++, name);
	GameObject* rawPtr = newGameObject.get();

	if (!newGameObject->Init())
	{
		return nullptr;
	}

	m_GameObjects.push_back(std::move(newGameObject));

	return rawPtr;
}

GameObject* Scene::GetGameObject(uint64_t uid)
{
	for (auto& go : m_GameObjects)
	{
		if (go->GetUID() == uid)
		{
			return go.get();
		}
	}

	return nullptr;
}

bool Scene::RemoveGameObject(uint64_t uid)
{
	GameObject* go = GetGameObject(uid);
	if (!go)
	{
		return false;
	}

	// Remove all children recursively
	while (!go->GetChildren().empty())
	{
		GameObject* child = go->GetChildren().back();
		if (!RemoveGameObject(child->GetUID()))
		{
			return false;
		}
	}

	// Detach from parent
	go->SetParent(nullptr);

	// Find it in the scene
	auto it = std::find_if(
		m_GameObjects.begin(),
		m_GameObjects.end(),
		[go](const std::unique_ptr<GameObject>& obj)
		{
			return obj.get() == go;
		}
	);

	if (it == m_GameObjects.end())
	{
		return false;
	}

	m_GameObjects.erase(it);
	return true;
}
