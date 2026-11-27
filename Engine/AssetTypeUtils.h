#pragma once

#include "AssetTypes.h"
#include <filesystem>

namespace AssetTypeUtils
{
	AssetType DetectAssetType(const std::filesystem::path& path);

	bool IsSupportedAsset(const std::filesystem::path& path);
}