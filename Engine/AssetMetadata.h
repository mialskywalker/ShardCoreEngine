#pragma once

#include "AssetTypes.h"

inline constexpr uint32_t AssetMetadataVersion = 1;

struct AssetMetadata
{
	uint32_t Version = AssetMetadataVersion;
	AssetID ID = InvalidAssetID;
	AssetType Type = AssetType::Unknown;

	bool IsValid() const
	{
		if (Version != AssetMetadataVersion || ID == InvalidAssetID || Type == AssetType::Unknown)
		{
			return false;
		}

		return true;
	}
};