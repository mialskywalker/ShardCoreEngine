#pragma once

#include "AssetRecord.h"
#include "AssetIDGenerator.h"
#include "AssetMetadataSerializer.h"
#include <unordered_map>
#include <cstddef>
#include <vector>

class FileSystem;

class AssetRegistry
{
public:
	struct DiscoveredAsset
	{
		std::filesystem::path SourcePath;
		AssetType Type = AssetType::Unknown;
	};

	explicit AssetRegistry(const FileSystem& fileSystem);

	AssetID CreateAsset(AssetType type, const std::string& name, const std::filesystem::path& sourcePath);
	AssetID LoadExistingAsset(const std::filesystem::path& sourcePath);
	AssetID RegisterNewAsset(const std::filesystem::path& sourcePath);
	bool DiscoverAssets(const std::filesystem::path& directory, std::vector<DiscoveredAsset>& outAssets) const;
	bool RegisterAsset(const AssetRecord& record);
	bool RemoveAsset(AssetID id);
	bool ScanAssets(const std::filesystem::path& directory);

	bool Contains(AssetID id) const;
	bool ContainsPath(const std::filesystem::path& sourcePath) const;

	AssetID GetAssetIDByPath(const std::filesystem::path& sourcePath) const;
	std::vector<AssetID> GetAssetIDsByPath(const std::filesystem::path& sourcePath) const;
	const AssetRecord* GetAsset(AssetID id) const;

	size_t GetAssetCount() const;

	void Clear();

private:
	AssetID GenerateUniqueID();
	static std::filesystem::path NormalizeAssetPath(const std::filesystem::path& path);

private:
	const FileSystem& m_FileSystem;
	AssetMetadataSerializer m_MetadataSerializer;
	AssetIDGenerator m_IDGenerator;
	std::unordered_map<AssetID, AssetRecord> m_Assets;
};