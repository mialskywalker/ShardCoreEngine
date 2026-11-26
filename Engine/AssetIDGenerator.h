#pragma once

#include "AssetTypes.h"
#include <random>

class AssetIDGenerator
{
public:
	AssetIDGenerator();

	AssetID Generate();

private:
	std::mt19937_64 m_Generator;
	std::uniform_int_distribution<AssetID> m_Distribution;
};