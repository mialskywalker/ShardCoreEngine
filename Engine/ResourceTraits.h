#pragma once

#include "AssetTypes.h"

class Mesh;

template<typename T>
struct ResourceTraits;

template<>
struct ResourceTraits<Mesh>
{
    static constexpr AssetType Type = AssetType::Mesh;
};