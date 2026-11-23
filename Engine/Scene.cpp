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
	m_IsUpdating = true;

	for (auto&& go : m_GameObjects)
	{
		if (go->IsActive())
		{
			go->Update(deltaTime);
		}
	}

	m_IsUpdating = false;

	// Process pending additions
	for (auto& go : m_PendingAdditions)
	{
		m_GameObjects.push_back(std::move(go));
	}

	m_PendingAdditions.clear();

	// Process pending removals
	std::vector<uint64_t> pending;
	pending.swap(m_PendingRemovals);
	
	for (uint64_t uid : pending)
	{
		RemoveGameObject(uid);
	}
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
	return CreateGameObjectWithUID(name, m_NextUID++);
}

GameObject* Scene::CreateGameObjectWithUID(const std::string& name, uint64_t uid)
{
	// check if UID is valid
	if (uid == 0 || uid >= std::numeric_limits<uint64_t>::max())
	{
		return nullptr;
	}

	// check if game object with this uid already exists
	GameObject* exists = GetGameObject(uid);
	if (exists)
	{
		return nullptr;
	}

	for (auto& go : m_PendingAdditions)
	{
		if (go->GetUID() == uid)
		{
			return nullptr;
		}
	}

	// create a new game object
	std::unique_ptr<GameObject> newGameObject = std::make_unique<GameObject>(this, uid, name);
	if (!newGameObject->Init())
	{
		return nullptr;
	}

	GameObject* rawPtr = newGameObject.get();
	
	// check if scene is updating
	if (m_IsUpdating)
	{
		m_PendingAdditions.push_back(std::move(newGameObject));
	}
	else
	{
		m_GameObjects.push_back(std::move(newGameObject));
	}

	m_NextUID = (std::max)(m_NextUID, uid + 1);

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

	// Defer removal during Scene Update
	if (m_IsUpdating)
	{

		if (std::find(m_PendingRemovals.begin(), m_PendingRemovals.end(), uid) == m_PendingRemovals.end())
		{
			m_PendingRemovals.push_back(uid);
		}

		return true;
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
