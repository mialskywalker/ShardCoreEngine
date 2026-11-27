#pragma once

#include <filesystem>
#include "AssetTypes.h"
#include "FileSystem.h"
#include <vector>
#include <string>

class EditorSelection;
class FileSystem;
class AssetRegistry;
class ResourceManager;

class WindowContentBrowser
{
public:
    WindowContentBrowser(const FileSystem& fileSystem, AssetRegistry& assetRegistry, ResourceManager& resourceManager, EditorSelection& selection);

    void Draw();

    bool IsOpen() const { return m_IsOpen; }
    void SetOpen (bool open) { m_IsOpen = open; }

private:
    enum class ViewMode
    {
        Grid,
        List
    };

    enum class FileAction { None, Rename, Delete, CreateFolder, Move, OpenFolder, Refresh, DropMove };

    void QueueAction(FileAction action, AssetID id = InvalidAssetID, const std::filesystem::path& path = {});
    void ProcessPendingAction();
    void DrawOperationModal();
    void DrawEntryContextMenu(const FileSystem::FileEntry& entry, AssetID id);
    void DrawBackgroundContextMenu();
    void HandleShortcuts();
    void ShowInExplorer(const std::filesystem::path& path, bool folder);
    bool IsSafeDirectory(const std::filesystem::path& path) const;
    bool CanModifyAsset(AssetID id, bool deleting) const;
    bool IsValidFileName(const std::string& name) const;
    void ReportError(const std::string& message);

    void DrawSearchControls();
    bool MatchesFilter(const FileSystem::FileEntry& entry, AssetID id) const;
    void DrawAssetDragSource(AssetID id);
    void DrawFolderDropTarget(const std::filesystem::path& folder);
    bool MoveAssetToFolder(AssetID id, const std::filesystem::path& folder);
    bool SynchronizeRegistry();
    bool RefreshDirectory();

    bool NavigateTo(const std::filesystem::path& directory);
    void NavigateUp();
    bool CanNavigateUp() const;

    void DrawNavigationBar();
    void DrawGridView();
    void DrawFolderTree();
    void DrawFolderNode(const std::filesystem::path& directory);
    void DrawBreadcrumbs();
    bool DrawAssetTile(const FileSystem::FileEntry& entry, AssetID assetID);
    void DrawDirectoryContents();
    void DrawListView();
    void DrawStatusBar();
    bool DrawAssetListItem(const FileSystem::FileEntry& entry, AssetID assetID);

    std::string TruncateText(const std::string& text, float maxWidth) const;

private:
    const FileSystem& m_FileSystem;
    AssetRegistry& m_AssetRegistry;
    ResourceManager& m_ResourceManager;
    std::vector<FileSystem::FileEntry> m_Entries;

    const std::filesystem::path m_RootDirectory = "Assets";
    std::filesystem::path m_CurrentDirectory = "Assets";

    EditorSelection& m_Selection;

    bool m_IsOpen = true;
    float m_ThumbnailSize = 66.0f;
    float m_FolderTreeWidth = 220.0f;

    float m_TilePadding = 8.0f;
    float m_TileSpacing = 10.0f;

    ViewMode m_ViewMode = ViewMode::Grid;

    size_t m_VisibleItemCount = 0;
    char m_Search[256] = {};
    AssetType m_TypeFilter = AssetType::Unknown;

    FileAction m_PendingAction = FileAction::None;
    AssetID m_PendingID = InvalidAssetID;
    std::filesystem::path m_PendingPath;

    FileAction m_ModalAction = FileAction::None;
    AssetID m_OperationID = InvalidAssetID;
    std::filesystem::path m_OperationDirectory;
    char m_OperationName[512] = {};
    std::string m_OperationError;
    std::string m_StatusError;

    float m_MinThumbnailSize = 48.0f;
    float m_MaxThumbnailSize = 192.0f;
};