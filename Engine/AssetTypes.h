#pragma once

#include <cstdint>

using AssetID = uint64_t;  // typedef uint64_t AssetID;

inline constexpr AssetID InvalidAssetID = 0;

enum class AssetType : uint8_t
{
	Unknown = 0,
	Texture = 1,
	Sprite = 2,
	Mesh = 3,
	AnimationClip = 4,
	AnimationGraph = 5,
	Material = 6,
	Audio = 7,
	Scene = 8,
	Prefab = 9,
	Model = 10
};