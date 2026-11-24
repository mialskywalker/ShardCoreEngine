#include "SceneSerializer.h"
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
		return false;
	}

	return true;
}

const std::unique_ptr<Scene> SceneSerializer::LoadScene(const std::filesystem::path& path)
{
	return std::unique_ptr<Scene>();
}


