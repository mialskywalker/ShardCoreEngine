#include "AssetRegistry.h"
#include "FileSystem.h"
#include "Logger.h"
#include "AssetTypeUtils.h"

AssetRegistry::AssetRegistry(const FileSystem& fileSystem)
	: m_FileSystem(fileSystem), m_MetadataSerializer(fileSystem)
{
}

AssetID AssetRegistry::CreateAsset(AssetType type, const std::string& name, const std::filesystem::path& sourcePath)
{
	if (type == AssetType::Unknown)
	{
		return InvalidAssetID;
	}

	if (name.empty())
	{
		return InvalidAssetID;
	}

	if (sourcePath.empty())
	{
		return InvalidAssetID;
	}

	if (sourcePath.is_absolute())
	{
		return InvalidAssetID;
	}

	std::filesystem::path normalizedPath = NormalizeAssetPath(sourcePath);
	if (normalizedPath.empty())
	{
		return InvalidAssetID;
	}

	if (ContainsPath(normalizedPath))
	{
		return InvalidAssetID;
	}

	AssetID id = GenerateUniqueID();
	AssetRecord record;
	record.ID = id;
	record.Type = type;
	record.Name = name;
	record.SourcePath = normalizedPath;
	
	if (!RegisterAsset(record))
	{
		return InvalidAssetID;
	}

	return id;
}

AssetID AssetRegistry::LoadExistingAsset(const std::filesystem::path& sourcePath)
{
	std::filesystem::path normalizedPath = NormalizeAssetPath(sourcePath);
	if (normalizedPath.empty())
	{
		return InvalidAssetID;
	}

	AssetID existingID = GetAssetIDByPath(normalizedPath);
	if (existingID != InvalidAssetID)
	{
		return existingID;
	}

	if (!m_FileSystem.IsRegularFile(normalizedPath))
	{
		return InvalidAssetID;
	}

	AssetType detectedType = AssetTypeUtils::DetectAssetType(normalizedPath);
	if (detectedType == AssetType::Unknown)
	{
		return InvalidAssetID;
	}

	// load metadata
	AssetMetadata metadata;
	if (!m_MetadataSerializer.LoadMetadata(normalizedPath, metadata))
	{
		return InvalidAssetID;
	}

	if (metadata.Type != detectedType)
	{
		return InvalidAssetID;
	}

	if (Contains(metadata.ID))
	{
		return InvalidAssetID;
	}

	// create assetrecord
	AssetRecord record;
	record.ID = metadata.ID;
	record.Type = metadata.Type;
	record.Name = normalizedPath.stem().string();
	record.SourcePath = normalizedPath;
	record.ParentAssetID = InvalidAssetID;

	if (!RegisterAsset(record))
	{
		return InvalidAssetID;
	}

	return record.ID;
}

AssetID AssetRegistry::RegisterNewAsset(const std::filesystem::path& sourcePath)
{
	std::filesystem::path normalizedPath = NormalizeAssetPath(sourcePath);
	if (normalizedPath.empty())
	{
		return InvalidAssetID;
	}

	AssetID existingID = GetAssetIDByPath(normalizedPath);
	if (existingID != InvalidAssetID)
	{
		return existingID;
	}

	if (!m_FileSystem.IsRegularFile(normalizedPath))
	{
		return InvalidAssetID;
	}

	AssetType detectedType = AssetTypeUtils::DetectAssetType(normalizedPath);
	if (detectedType == AssetType::Unknown)
	{
		return InvalidAssetID;
	}

	std::filesystem::path metadataPath = AssetMetadataSerializer::GetMetadataPath(normalizedPath);
	if (m_FileSystem.Exists(metadataPath))
	{
		return InvalidAssetID;
	}

	std::string name = normalizedPath.stem().string();
	AssetID id = CreateAsset(detectedType, name, normalizedPath);
	if (id == InvalidAssetID)
	{
		return InvalidAssetID;
	}

	AssetMetadata metadata;
	metadata.Version = AssetMetadataVersion;
	metadata.ID = id;
	metadata.Type = detectedType;

	if (!m_MetadataSerializer.SaveMetadata(normalizedPath, metadata))
	{
		RemoveAsset(id);
		return InvalidAssetID;
	}

	return id;
}

AssetID AssetRegistry::CreateSubAsset(AssetID parentID,	AssetType type,	const std::string& name)
{
	const AssetRecord* parent = GetAsset(parentID);

	if (!parent || type == AssetType::Unknown || name.empty())
	{
		return InvalidAssetID;
	}

	for (const auto& [id, record] : m_Assets)
	{
		if (record.ParentAssetID == parentID &&
			record.Name == name &&
			record.Type == type)
		{
			return InvalidAssetID;
		}
	}

	AssetRecord record;

	record.ID = GenerateUniqueID();
	record.Type = type;
	record.Name = name;
	record.SourcePath = parent->SourcePath;
	record.ParentAssetID = parentID;

	if (!RegisterAsset(record))
	{
		return InvalidAssetID;
	}

	return record.ID;
}

bool AssetRegistry::DiscoverAssets(const std::filesystem::path& directory, std::vector<DiscoveredAsset>& outAssets) const
{
	outAssets.clear();

	if (directory.empty())
	{
		return false;
	}

	std::vector<std::filesystem::path> files;
	if (!m_FileSystem.ListFilesRecursive(directory, files))
	{
		return false;
	}

	for (const auto& file : files)
	{
		AssetType type = AssetTypeUtils::DetectAssetType(file);
		if (type == AssetType::Unknown)
		{
			continue;
		}

		DiscoveredAsset asset;
		asset.SourcePath = file;
		asset.Type = type;

		outAssets.push_back(asset);
	}

	return true;
}

bool AssetRegistry::RegisterAsset(const AssetRecord& record)
{
	if (!record.IsValid())
	{
		return false;
	}

	if (Contains(record.ID))
	{
		return false;
	}

	if (record.ParentAssetID == record.ID)
	{
		return false;
	}

	if (record.IsSubAsset())
	{
		const AssetRecord* parent =	GetAsset(record.ParentAssetID);

		if (!parent)
		{
			return false;
		}

		if (parent->SourcePath != NormalizeAssetPath(record.SourcePath))
		{
			return false;
		}
	}

	AssetRecord rec = record;
	std::filesystem::path normalizedPath = NormalizeAssetPath(rec.SourcePath);
	if (normalizedPath.empty())
	{
		return false;
	}

	rec.SourcePath = normalizedPath;
	if (!rec.IsSubAsset())
	{
		if (ContainsPath(rec.SourcePath))
		{
			return false;
		}
	}

	auto res = m_Assets.emplace(rec.ID, rec);
	if (res.second)
	{
		return true;
	}

	return false;
}

bool AssetRegistry::Contains(AssetID id) const
{
	if (id == InvalidAssetID)
	{
		return false;
	}

	return m_Assets.find(id) != m_Assets.end();
}

bool AssetRegistry::ContainsPath(const std::filesystem::path& sourcePath) const
{
	if (GetAssetIDByPath(sourcePath) != InvalidAssetID)
	{
		return true;
	}

	return false;
}

AssetID AssetRegistry::GetAssetIDByPath(const std::filesystem::path& sourcePath) const
{
	std::filesystem::path normalizedPath = NormalizeAssetPath(sourcePath);
	if (normalizedPath.empty())
	{
		return InvalidAssetID;
	}

	for (const auto& [id, record] : m_Assets)
	{
		if (record.IsSubAsset())
		{
			continue;
		}

		if (record.SourcePath == normalizedPath)
		{
			return record.ID;
		}
	}

	return InvalidAssetID;
}

std::vector<AssetID> AssetRegistry::GetAssetIDsByPath(const std::filesystem::path& sourcePath) const
{
	std::vector<AssetID> assetIDs;
	std::filesystem::path normalizedPath = NormalizeAssetPath(sourcePath);
	if (normalizedPath.empty())
	{
		return assetIDs;
	}

	for (const auto& [id, record] : m_Assets)
	{
		if (record.SourcePath == normalizedPath)
		{
			assetIDs.push_back(record.ID);
		}
	}

	return assetIDs;
}

const AssetRecord* AssetRegistry::GetAsset(AssetID id) const
{
	auto it = m_Assets.find(id);
	if (it == m_Assets.end())
	{
		return nullptr;
	}

	return &it->second;
}

std::vector<AssetRecord> AssetRegistry::GetAllAssets() const
{
	std::vector<AssetRecord> assets;
	assets.reserve(m_Assets.size());

	for (const auto& [id, record] : m_Assets)
	{
		assets.push_back(record);
	}

	return assets;
}

std::vector<AssetID> AssetRegistry::GetSubAssets(AssetID parentID) const
{
	std::vector<AssetID> subAssets;

	if (parentID == InvalidAssetID || !Contains(parentID))
	{
		return subAssets;
	}

	for (const auto& [id, record] : m_Assets)
	{
		if (record.ParentAssetID == parentID)
		{
			subAssets.push_back(id);
		}
	}

	return subAssets;
}

bool AssetRegistry::RemoveAsset(AssetID id)
{
	if (m_Assets.erase(id))
	{
		return true;
	}

	return false;
}

bool AssetRegistry::DeleteAsset(AssetID id)
{
	if (id == InvalidAssetID)
	{
		return false;
	}

	const AssetRecord* record = GetAsset(id);
	if (!record)
	{
		return false;
	}

	if (record->IsSubAsset())
	{
		return false;
	}

	std::filesystem::path sourcePath = record->SourcePath;
	std::filesystem::path metadataPath = AssetMetadataSerializer::GetMetadataPath(sourcePath);

	if (!m_FileSystem.IsRegularFile(sourcePath) || !m_FileSystem.IsRegularFile(metadataPath))
	{
		return false;
	}

	std::vector<AssetID> relatedAssetIDs = GetAssetIDsByPath(sourcePath);
	if (!m_FileSystem.RemoveFile(sourcePath))
	{
		return false;
	}

	bool metadataRemoved = m_FileSystem.RemoveFile(metadataPath);
	if (!metadataRemoved)
	{
		Logger::Error("Metadata could not be removed", LogType::Engine);
	}

	bool recordsRemoved = true;
	for (auto assetID : relatedAssetIDs)
	{
		if (!RemoveAsset(assetID))
		{
			recordsRemoved = false;
		}
	}

	return metadataRemoved && recordsRemoved;
}

bool AssetRegistry::MoveAsset(AssetID id, const std::filesystem::path& destinationPath)
{
	const AssetRecord* record = GetAsset(id);

	if (!record || record->IsSubAsset())
	{
		return false;
	}

	const auto sourcePath = record->SourcePath;
	const auto normalizedPath =	NormalizeAssetPath(destinationPath);

	if (normalizedPath.empty() || sourcePath == normalizedPath)
	{
		return false;
	}

	if (ContainsPath(normalizedPath))
	{
		return false;
	}

	// Preserve asset type and file format
	if (AssetTypeUtils::DetectAssetType(normalizedPath)	!= record->Type 
		||	sourcePath.extension() != normalizedPath.extension())
	{
		return false;
	}

	const auto sourceMetaPath =	AssetMetadataSerializer::GetMetadataPath(sourcePath);
	const auto destinationMetaPath = AssetMetadataSerializer::GetMetadataPath(normalizedPath);

	if (!m_FileSystem.IsRegularFile(sourcePath) || !m_FileSystem.IsRegularFile(sourceMetaPath))
	{
		return false;
	}

	if (m_FileSystem.Exists(normalizedPath) || m_FileSystem.Exists(destinationMetaPath))
	{
		return false;
	}

	// Verify metadata identity before moving
	AssetMetadata metadata;

	if (!m_MetadataSerializer.LoadMetadata(sourcePath, metadata) 
		|| metadata.ID != id || metadata.Type != record->Type)
	{
		return false;
	}

	const auto relatedAssetIDs = GetAssetIDsByPath(sourcePath);
	const std::string newName =	normalizedPath.stem().string();

	// Move source file
	if (!m_FileSystem.RelocateFile(sourcePath, normalizedPath))
	{
		return false;
	}

	// Move metadata file
	if (!m_FileSystem.RelocateFile(sourceMetaPath, destinationMetaPath))
	{
		// Rollback source file
		if (!m_FileSystem.RelocateFile(normalizedPath, sourcePath))
		{
			Logger::Error(
				"[AssetRegistry] Move rollback failed. "
				"Manual recovery required.",
				LogType::Engine);
		}

		return false;
	}

	// Update registered asset paths
	for (AssetID assetID : relatedAssetIDs)
	{
		auto it = m_Assets.find(assetID);

		if (it != m_Assets.end())
		{
			it->second.SourcePath = normalizedPath;

			if (assetID == id)
			{
				it->second.Name = newName;
			}
		}
	}

	return true;
}

bool AssetRegistry::RenameAsset(AssetID id, const std::string& newName)
{
	const AssetRecord* record = GetAsset(id);

	if (!record || record->IsSubAsset())
	{
		return false;
	}

	if (newName.empty() || newName == "." || newName == "..")
	{
		return false;
	}

	// Reject invalid Windows filename characters
	if (newName.find_first_of("\\/:*?\"<>|") !=	std::string::npos)
	{
		return false;
	}

	if (newName.back() == '.' || newName.back() == ' ')
	{
		return false;
	}

	const auto destinationPath = record->SourcePath.parent_path() /	(newName + record->SourcePath.extension().string());

	return MoveAsset(id, destinationPath);
}

bool AssetRegistry::ScanAssets(const std::filesystem::path& directory)
{
	std::vector<DiscoveredAsset> discoveredAssets;
	if (!DiscoverAssets(directory, discoveredAssets))
	{
		return false;
	}

	std::vector<DiscoveredAsset> newAssets;
	bool success = true;
	size_t loadedCount = 0;
	size_t createdCount = 0;
	size_t failedCount = 0;
	size_t skippedCount = 0;

	for (const DiscoveredAsset& asset : discoveredAssets)
	{
		if (ContainsPath(asset.SourcePath))
		{
			skippedCount++;
			continue;
		}

		std::filesystem::path metadataPath = AssetMetadataSerializer::GetMetadataPath(asset.SourcePath);
		if (!m_FileSystem.Exists(metadataPath))
		{
			newAssets.push_back(asset);
			continue;
		}

		AssetID id = LoadExistingAsset(asset.SourcePath);
		if (id == InvalidAssetID)
		{
			success = false;
			failedCount++;
			continue;
		}

		loadedCount++;
	}

	if (!success)
	{
		return false;
	}

	for (const DiscoveredAsset& asset : newAssets)
	{
		AssetID id = RegisterNewAsset(asset.SourcePath);
		if (id == InvalidAssetID)
		{
			success = false;
			failedCount++;
			continue;
		}

		createdCount++;
	}

	Logger::Log("[AssetRegistry] Asset scan completed.", LogType::Engine);

	Logger::Log("[AssetRegistry] Discovered: " +
		std::to_string(discoveredAssets.size()), LogType::Engine);

	Logger::Log("[AssetRegistry] Loaded: " +
		std::to_string(loadedCount), LogType::Engine);

	Logger::Log("[AssetRegistry] Created: " +
		std::to_string(createdCount), LogType::Engine);

	Logger::Log("[AssetRegistry] Skipped: " +
		std::to_string(skippedCount), LogType::Engine);

	Logger::Log("[AssetRegistry] Failed: " +
		std::to_string(failedCount), LogType::Engine);

	Logger::Log("[AssetRegistry] Total registered assets: " +
		std::to_string(GetAssetCount()), LogType::Engine);

	return success;

}

size_t AssetRegistry::GetAssetCount() const
{
	return m_Assets.size();
}

void AssetRegistry::Clear()
{
	m_Assets.clear();
}

AssetID AssetRegistry::GenerateUniqueID()
{
	AssetID id = InvalidAssetID;
	do
	{
		id = m_IDGenerator.Generate();
	} while (id == InvalidAssetID || Contains(id));

	return id;
}

std::filesystem::path AssetRegistry::NormalizeAssetPath(const std::filesystem::path& path)
{
	if (path.empty())
	{
		return std::filesystem::path();
	}

	if (path.is_absolute() || path.has_root_name() || path.has_root_directory())
	{
		return std::filesystem::path();
	}

	std::filesystem::path normalizedPath = path.lexically_normal();
	if (normalizedPath.empty() || normalizedPath == "." || *normalizedPath.begin() == "..")
	{
		return std::filesystem::path();
	}

	return normalizedPath;
}
