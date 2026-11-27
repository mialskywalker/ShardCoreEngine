#pragma once

#include "AssetRegistry.h"
#include "ResourceTraits.h"

#include <memory>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <utility>

class ResourceManager
{
public:
    explicit ResourceManager(const AssetRegistry& assetRegistry);

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    template<typename T>
    using Loader = std::function<std::shared_ptr<T>(const AssetRecord&)>;

    template<typename T>
    bool RegisterLoader(Loader<T> loader);

    template<typename T>
    std::shared_ptr<T> Load(AssetID id);

    template<typename T>
    std::shared_ptr<T> Get(AssetID id) const;

    bool IsLoaded(AssetID id) const;

    bool Unload(AssetID id);

    size_t UnloadUnused();

    size_t GetLoadedResourceCount() const;

    void Clear();

private:
    struct CachedResource
    {
        std::shared_ptr<void> Resource;
        std::type_index Type;
    };

    using ResourceLoader = std::function<std::shared_ptr<void>(const AssetRecord&)>;

private:
    const AssetRegistry& m_AssetRegistry;
    std::unordered_map<AssetID, CachedResource> m_Resources;
    std::unordered_map<std::type_index, ResourceLoader> m_Loaders;
};

template<typename T>
bool ResourceManager::RegisterLoader(Loader<T> loader)
{
    if (!loader)
    {
        return false;
    }

    const std::type_index type = typeid(T);

    if (m_Loaders.find(type) != m_Loaders.end())
    {
        return false;
    }

    ResourceLoader erasedLoader = [loader = std::move(loader)](const AssetRecord& record)-> std::shared_ptr<void>
        {
            return loader(record);
        };

    return m_Loaders.emplace(type, std::move(erasedLoader)).second;
}

template<typename T>
std::shared_ptr<T> ResourceManager::Get(AssetID id) const
{
    const AssetRecord* record = m_AssetRegistry.GetAsset(id);

    if (!record || record->Type != ResourceTraits<T>::Type)
    {
        return nullptr;
    }

    auto it = m_Resources.find(id);

    if (it == m_Resources.end())
    {
        return nullptr;
    }

    if (it->second.Type != std::type_index(typeid(T)))
    {
        return nullptr;
    }

    return std::static_pointer_cast<T>(it->second.Resource);
}

template<typename T>
std::shared_ptr<T> ResourceManager::Load(AssetID id)
{
    if (id == InvalidAssetID)
    {
        return nullptr;
    }

    const AssetRecord* record = m_AssetRegistry.GetAsset(id);

    if (!record)
    {
        return nullptr;
    }

    if (record->Type != ResourceTraits<T>::Type)
    {
        return nullptr;
    }

    // Return existing resource
    if (auto resource = Get<T>(id))
    {
        return resource;
    }

    // Find loader
    const std::type_index type = typeid(T);

    auto loaderIt = m_Loaders.find(type);

    if (loaderIt == m_Loaders.end())
    {
        return nullptr;
    }

    // Load resource
    auto resource = loaderIt->second(*record);

    if (!resource)
    {
        return nullptr;
    }

    // Add to cache
    auto result = m_Resources.emplace(id, CachedResource{ resource, type });

    if (result.first->second.Type != type)
    {
        return nullptr;
    }

    return std::static_pointer_cast<T>(result.first->second.Resource);
}