#pragma once

#include "AssetTypes.h"
#include <string>
#include <filesystem>

struct AssetRecord
{
	AssetID ID = InvalidAssetID;
	AssetType Type = AssetType::Unknown;
	std::string Name;
	std::filesystem::path SourcePath;
	std::filesystem::path ImportedPath;
	AssetID ParentAssetID = InvalidAssetID;

	bool IsValid() const
	{
		if (ID == InvalidAssetID || Type == AssetType::Unknown)
		{
			return false;
		}

		return true;
	};

	bool IsSubAsset() const
	{
		if (ParentAssetID == InvalidAssetID)
		{
			return false;
		}

		return true;
	};
};