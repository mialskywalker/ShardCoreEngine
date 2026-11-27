#include "AssetTypeUtils.h"
#include <string>
#include <algorithm>
#include <cctype>

AssetType AssetTypeUtils::DetectAssetType(const std::filesystem::path& path)
{
	if (path.empty())
	{
		return AssetType::Unknown;
	}

	if (!path.has_extension())
	{
		return AssetType::Unknown;
	}

	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (extension == ".png"
		|| extension == ".jpg"
		|| extension == ".jpeg"
		|| extension == ".dds"
		|| extension == ".bmp"
		|| extension == ".tga")
	{
		return AssetType::Texture;
	}
	else if (extension == ".gltf" || extension == ".glb")
	{
		return AssetType::Model;
	}
	else if (extension == ".shardmesh")
	{
		return AssetType::Mesh;
	}
	else if (extension == ".anim")
	{
		return AssetType::AnimationClip;
	}
	else if (extension == ".animgraph")
	{
		return AssetType::AnimationGraph;
	}
	else if (extension == ".wav"
		|| extension == ".ogg"
		|| extension == ".mp3")
	{
		return AssetType::Audio;
	}

	return AssetType::Unknown;
}

bool AssetTypeUtils::IsSupportedAsset(const std::filesystem::path& path)
{
	if (DetectAssetType(path) != AssetType::Unknown)
	{
		return true;
	}

	return false;
}
