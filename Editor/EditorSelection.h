#pragma once
#include "AssetTypes.h"
#include <cstdint>
#include <filesystem>

enum class EditorSelectionType { None, GameObject, Asset, Folder };

class EditorSelection
{
public:
    EditorSelectionType GetType() const { return m_Type; }
    uint64_t GetGameObjectUID() const { return m_UID; }
    AssetID GetAssetID() const { return m_AssetID; }
    const std::filesystem::path& GetFolder() const { return m_Folder; }

    void Clear()
    {
        m_Type = EditorSelectionType::None;
        m_UID = 0;
        m_AssetID = InvalidAssetID;
        m_Folder.clear();
    }

    void SelectGameObject(uint64_t uid)
    {
        Clear();
        if (uid == 0) return;
        m_UID = uid;
        m_Type = EditorSelectionType::GameObject;
    }

    void SelectAsset(AssetID id)
    {
        Clear();
        if (id == InvalidAssetID) return;
        m_AssetID = id;
        m_Type = EditorSelectionType::Asset;
    }

    void SelectFolder(const std::filesystem::path& path)
    {
        auto normalized = path.lexically_normal();
        Clear();
        if (normalized.empty()) return;
        m_Folder = normalized;
        m_Type = EditorSelectionType::Folder;
    }

    void ClearContentSelection()
    {
        if (m_Type == EditorSelectionType::Asset ||
            m_Type == EditorSelectionType::Folder)
            Clear();
    }

private:
    EditorSelectionType m_Type = EditorSelectionType::None;
    uint64_t m_UID = 0;
    AssetID m_AssetID = InvalidAssetID;
    std::filesystem::path m_Folder;
};
