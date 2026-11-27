#include "ResourceManager.h"

ResourceManager::ResourceManager(
    const AssetRegistry& assetRegistry)
    : m_AssetRegistry(assetRegistry)
{
}

bool ResourceManager::IsLoaded(AssetID id) const
{
    return m_Resources.find(id) != m_Resources.end();
}

bool ResourceManager::Unload(AssetID id)
{
    return m_Resources.erase(id) > 0;
}

size_t ResourceManager::UnloadUnused()
{
    size_t removedCount = 0;

    auto it = m_Resources.begin();

    while (it != m_Resources.end())
    {
        if (it->second.Resource.use_count() == 1)
        {
            it = m_Resources.erase(it);
            removedCount++;
        }
        else
        {
            ++it;
        }
    }

    return removedCount;
}

size_t ResourceManager::GetLoadedResourceCount() const
{
    return m_Resources.size();
}

void ResourceManager::Clear()
{
    m_Resources.clear();
}