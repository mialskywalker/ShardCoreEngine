#include "AssetMetadataSerializer.h"
#include "FileSystem.h"
#include "Logger.h"
#include <json.hpp>

AssetMetadataSerializer::AssetMetadataSerializer(const FileSystem& fileSystem)
	: m_FileSystem(fileSystem)
{
}

bool AssetMetadataSerializer::SaveMetadata(const std::filesystem::path& sourcePath, const AssetMetadata& metadata) const
{
	if (sourcePath.empty())
	{
		Logger::Log("[AssetMetadataSerializer] Invalid sourcePath", LogType::Engine);
		return false;
	}

	if (!m_FileSystem.IsRegularFile(sourcePath))
	{
		Logger::Log("[AssetMetadataSerializer] Source file doesn't exist!", LogType::Engine);
		return false;
	}

	if (!metadata.IsValid())
	{
		Logger::Log("[AssetMetadataSerializer] Invalid metadata file.", LogType::Engine);
		return false;
	}

	std::string typeString = AssetTypeToString(metadata.Type);
	if (typeString == "Unknown")
	{
		return false;
	}

	std::filesystem::path metadataPath = GetMetadataPath(sourcePath);
	if (metadataPath.empty())
	{
		return false;
	}

	nlohmann::json obj;
	obj["version"] = metadata.Version;
	obj["id"] = metadata.ID;
	obj["type"] = typeString;

	std::string objText = obj.dump(4);
	if (!m_FileSystem.WriteTextFile(metadataPath, objText))
	{
		return false;
	}

	return true;
}

bool AssetMetadataSerializer::LoadMetadata(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata) const
{
	if (sourcePath.empty())
	{
		Logger::Log("[AssetMetadataSerializer] Invalid sourcePath", LogType::Engine);
		return false;
	}

	if (!m_FileSystem.IsRegularFile(sourcePath))
	{
		Logger::Log("[AssetMetadataSerializer] Source file doesn't exist!", LogType::Engine);
		return false;
	}

	std::filesystem::path metadataPath = GetMetadataPath(sourcePath);
	if (metadataPath.empty())
	{
		Logger::Log("[AssetMetadataSerializer] Invalid metadata path!", LogType::Engine);
		return false;
	}

	std::string objText;
	if (!m_FileSystem.ReadTextFile(metadataPath, objText))
	{
		Logger::Log("[AssetMetadataSerializer] Cannot read from metadata file!", LogType::Engine);
		return false;
	}

	nlohmann::json obj;
	try
	{
		obj = nlohmann::json::parse(objText);

		if (!obj.contains("version") || !obj.contains("id") || !obj.contains("type"))
		{
			return false;
		}

		if (!obj["version"].is_number_unsigned() || !obj["id"].is_number_unsigned() || !obj["type"].is_string())
		{
			return false;
		}

		if (obj["version"] != AssetMetadataVersion)
		{
			return false;
		}

		AssetMetadata metadata;
		metadata.Version = obj["version"].get<uint32_t>();
		metadata.ID = obj["id"].get<AssetID>();
		metadata.Type = StringToAssetType(obj["type"].get<std::string>());
	
		if (!metadata.IsValid())
		{
			return false;
		}

		outMetadata = metadata;
		return true;
	}
	catch (const nlohmann::json::parse_error& e)
	{
		Logger::Error(e.what(), LogType::Engine);
		return false;
	}
}

std::filesystem::path AssetMetadataSerializer::GetMetadataPath(const std::filesystem::path& sourcePath)
{
	if (sourcePath.empty())
	{
		return std::filesystem::path();
	}

	std::filesystem::path path = sourcePath;
	path += ".meta";

	return path;
}

std::string AssetMetadataSerializer::AssetTypeToString(AssetType type)
{
	switch (type)
	{
	case AssetType::Unknown:
		return "Unknown";
	case AssetType::Texture:
		return "Texture";
	case AssetType::Sprite:
		return "Sprite";
	case AssetType::Mesh:
		return "Mesh";
	case AssetType::AnimationClip:
		return "AnimationClip";
	case AssetType::AnimationGraph:
		return "AnimationGraph";
	case AssetType::Material:
		return "Material";
	case AssetType::Audio:
		return "Audio";
	case AssetType::Scene:
		return "Scene";
	case AssetType::Prefab:
		return "Prefab";
	case AssetType::Model:
		return "Model";
	default:
		return "Unknown";
	}
}

AssetType AssetMetadataSerializer::StringToAssetType(const std::string& type)
{
	if (type == "Unknown") return AssetType::Unknown;
	else if (type == "Texture") return AssetType::Texture;
	else if (type == "Sprite") return AssetType::Sprite;
	else if (type == "Mesh") return AssetType::Mesh;
	else if (type == "AnimationClip") return AssetType::AnimationClip;
	else if (type == "AnimationGraph") return AssetType::AnimationGraph;
	else if (type == "Material") return AssetType::Material;
	else if (type == "Audio") return AssetType::Audio;
	else if (type == "Scene") return AssetType::Scene;
	else if (type == "Prefab") return AssetType::Prefab;
	else if (type == "Model") return AssetType::Model;
	else return AssetType::Unknown;
}

