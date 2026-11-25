#include "SceneSerializer.h"
#include "Logger.h"
#include "FileSystem.h"
#include "Scene.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include <json.hpp>
#include <string>

SceneSerializer::SceneSerializer(const FileSystem& fs)
	: m_FileSystem(fs)
{
}

const bool SceneSerializer::SaveScene(const Scene& scene, const std::filesystem::path& path)
{
	nlohmann::json saveData;
	saveData["version"] = 1;
	saveData["name"] = scene.GetSceneName();
	saveData["gameObjects"] = nlohmann::json::array();

	// game objects
	for (auto& go : scene.GetGameObjects())
	{
		nlohmann::json object;
		object["uid"] = go->GetUID();
		object["name"] = go->GetName();
		object["active"] = go->IsActive();
		
		GameObject* parent = go->GetParent();
		if (parent)
		{
			object["parentUID"] = parent->GetUID();
		}
		else
		{
			object["parentUID"] = nullptr;
		}

		object["components"] = nlohmann::json::array();

		// components
			// transform
		TransformComponent* transform = go->GetTransform();
		nlohmann::json transformObject;
		transformObject["type"] = "Transform";
		transformObject["enabled"] = transform->IsEnabled();

		Vector3 position = transform->GetPosition();
		Quaternion rotation = transform->GetRotation();
		Vector3 scale = transform->GetScale();
		transformObject["position"] = { position.x, position.y, position.z };
		transformObject["rotation"] = { rotation.x, rotation.y, rotation.z, rotation.w };
		transformObject["scale"] = { scale.x, scale.y, scale.z };

		object["components"].push_back(transformObject);

		saveData["gameObjects"].push_back(object);
	}

	std::string data = saveData.dump(4);
	if (!m_FileSystem.WriteTextFile(path, data))
	{
		Logger::Error("Could not save scene: " + scene.GetSceneName(), LogType::Engine);
		return false;
	}

	Logger::Log(scene.GetSceneName() + " saved successfully.", LogType::Engine);
	return true;
}

std::unique_ptr<Scene> SceneSerializer::LoadScene(const std::filesystem::path& path)
{
	std::string data;
	if (!m_FileSystem.ReadTextFile(path, data))
	{
		Logger::Error("Could not load scene!", LogType::Engine);
		return nullptr;
	}

	nlohmann::json loadData;
	
	try
	{
		loadData = nlohmann::json::parse(data);
	}
	catch (const nlohmann::json::parse_error& e)
	{
		Logger::Error(e.what(), LogType::Engine);
		return nullptr;
	}

	if (!loadData.is_object())
	{
		return nullptr;
	}

	if (!loadData.contains("version") || !loadData["version"].is_number_integer() || loadData["version"] != 1)
	{
		return nullptr;
	}

	if (!loadData.contains("name") || !loadData["name"].is_string())
	{
		return nullptr;
	}

	if (!loadData.contains("gameObjects") || !loadData["gameObjects"].is_array())
	{
		return nullptr;
	}

	// proceed to scene creation
	std::string sceneName = loadData["name"];
	std::unique_ptr<Scene> newScene = std::make_unique<Scene>(sceneName);
	if (!newScene->Init())
	{
		return nullptr;
	}

	// game object deserialization
	if (!loadData["gameObjects"].is_array())
	{
		return nullptr;
	}
	
	for (auto go : loadData["gameObjects"])
	{
		if (!go.is_object())
		{
			return nullptr;
		}

		if (!go.contains("uid") || !go["uid"].is_number_integer())
		{
			return nullptr;
		}

		if (!go.contains("name") || !go["name"].is_string())
		{
			return nullptr;
		}

		if (!go.contains("active") || !go["active"].is_boolean())
		{
			return nullptr;
		}

		if (!go.contains("parentUID") || (!go["parentUID"].is_number_unsigned() && !go["parentUID"].is_null()))
		{
			return nullptr;
		}

		uint64_t goUID = go["uid"];
		std::string goName = go["name"];
		bool goActive = go["active"];

		GameObject* gameObject = newScene->CreateGameObjectWithUID(goName, goUID);
		if (!gameObject)
		{
			return nullptr;
		}
		gameObject->SetActive(goActive);
	}

	// component deserialization
	for (auto go : loadData["gameObjects"])
	{
		GameObject* gameObject = newScene->GetGameObject(go["uid"]);
		if (!gameObject)
		{
			return nullptr;
		}

		if (!go.contains("components") || !go["components"].is_array())
		{
			return nullptr;
		}

		for (auto comp : go["components"])
		{
			if (!comp.is_object())
			{
				return nullptr;
			}

			if (!comp.contains("type") || !comp["type"].is_string())
			{
				return nullptr;
			}

			if (comp["type"] == "Transform")
			{
				if (!comp.contains("enabled") || !comp["enabled"].is_boolean())
				{
					return nullptr;
				}

				if (!comp.contains("position") || (!comp["position"].is_array() && !comp["position"].size() == 3))
				{
					return nullptr;
				}

				if (!comp.contains("rotation") || (!comp["rotation"].is_array() && !comp["rotation"].size() == 4))
				{
					return nullptr;
				}

				if (!comp.contains("scale") || (!comp["scale"].is_array() && !comp["scale"].size() == 3))
				{
					return nullptr;
				}

				TransformComponent* t = gameObject->GetTransform();
				Vector3 position = Vector3(comp["position"][0], comp["position"][1], comp["position"][2]);
				Quaternion rotation = Quaternion(comp["rotation"][0], comp["rotation"][1], comp["rotation"][2], comp["rotation"][3]);
				Vector3 scale = Vector3(comp["scale"][0], comp["scale"][1], comp["scale"][2]);
				bool enabled = comp["enabled"];
				
				t->SetPosition(position);
				t->SetRotation(rotation);
				t->SetScale(scale);
				t->SetEnabled(enabled);
			}
		}
	}

	// set parent
	for (auto go : loadData["gameObjects"])
	{
		GameObject* gameObject = newScene->GetGameObject(go["uid"]);
		if (!gameObject)
		{
			return nullptr;
		}

		if (go["parentUID"].is_null())
		{
			continue;
		}

		uint64_t uid = go["parentUID"];
		GameObject* parent = newScene->GetGameObject(uid);
		if (!parent)
		{
			return nullptr;
		}

		if (!gameObject->SetParent(parent))
		{
			return nullptr;
		}
	}

	Logger::Log(newScene->GetSceneName() + " scene loaded successfully!", LogType::Engine);
	return newScene;
}


