#pragma once

#include "AssetMetadata.h"
#include <filesystem>
#include <string>

class FileSystem;

class AssetMetadataSerializer
{
public:
	explicit AssetMetadataSerializer(const FileSystem& filesystem);

	bool SaveMetadata(const std::filesystem::path& sourcePath, const AssetMetadata& metadata) const;
	bool LoadMetadata(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata) const;

	static std::filesystem::path GetMetadataPath(const std::filesystem::path& sourcePath);

private:
	static std::string AssetTypeToString(AssetType type);
	static AssetType StringToAssetType(const std::string& type);

private:
	const FileSystem& m_FileSystem;
};