#include "AssetIDGenerator.h"
#include <limits>

AssetIDGenerator::AssetIDGenerator()
	: m_Generator(std::random_device{}()), m_Distribution(1, std::numeric_limits<AssetID>::max())
{
}

AssetID AssetIDGenerator::Generate()
{
	return m_Distribution(m_Generator);
}
