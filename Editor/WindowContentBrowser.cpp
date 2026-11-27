#include "WindowContentBrowser.h"
#include "EditorSelection.h"
#include "FileSystem.h"
#include "AssetRegistry.h"
#include "ResourceManager.h"
#include "EditorIcons.h"
#include "AssetVisuals.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "imgui.h"
#include "AssetMetadataSerializer.h"
#include "AssetTypeUtils.h"
#include <Windows.h>
#include <shellapi.h>
#include <cstdio>
#include <cctype>
#include <utility>
#include <exception>
#include <string_view>
#include <cstring>
#pragma comment(lib, "Shell32.lib")

namespace
{
    std::filesystem::path PathFromUTF8(std::string_view text)
    {
        std::u8string utf8(text.size(), u8'\0');
        if (!text.empty())
        {
            std::memcpy(utf8.data(), text.data(), text.size());
        }

        return std::filesystem::path(utf8);
    }

    std::string PathToUTF8(const std::filesystem::path& path)
    {
        const auto utf8 = path.u8string();
        return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
    }
}


WindowContentBrowser::WindowContentBrowser(const FileSystem& fileSystem, AssetRegistry& assetRegistry, ResourceManager& resourceManager, EditorSelection& selection)
    : m_FileSystem(fileSystem), m_AssetRegistry(assetRegistry), m_ResourceManager(resourceManager), m_Selection(selection)
{
    RefreshDirectory();
}

void WindowContentBrowser::Draw()
{
    if (!m_IsOpen)
    {
        return;
    }

    if (ImGui::Begin("Content Browser", &m_IsOpen))
    {
        DrawNavigationBar();
        DrawSearchControls();

        const float statusBarHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2.0f;

        // Folder Tree
        if (ImGui::BeginChild("FolderTree", ImVec2(m_FolderTreeWidth, -statusBarHeight), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            DrawFolderTree();
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // Directory Contents
        if (ImGui::BeginChild("DirectoryContents", ImVec2(0.0f, -statusBarHeight), ImGuiChildFlags_Borders))
        {
            DrawDirectoryContents();
        }

        ImGui::EndChild();

        DrawStatusBar();
        HandleShortcuts();
        ProcessPendingAction();
        DrawOperationModal();
        if (!m_StatusError.empty() && m_ModalAction == FileAction::None)
        {
            ImGui::OpenPopup("Content Browser Error");
        }
        if (ImGui::BeginPopupModal("Content Browser Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("%s", m_StatusError.c_str());
            if (ImGui::Button("OK"))
            {
                m_StatusError.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

bool WindowContentBrowser::RefreshDirectory()
{
    if (!IsSafeDirectory(m_CurrentDirectory)) return false;
    std::vector<FileSystem::FileEntry> entries;

    if (!m_FileSystem.ListDirectory(
        m_CurrentDirectory, entries))
    {
        return false;
    }

    m_Entries = std::move(entries);

    return true;
}

bool WindowContentBrowser::NavigateTo(
    const std::filesystem::path& directory)
{
    if (directory.empty() ||
        directory.has_root_name() ||
        directory.has_root_directory())
    {
        return false;
    }

    const auto normalizedPath =
        directory.lexically_normal();

    if (!IsSafeDirectory(normalizedPath)) return false;

    const auto relativePath =
        normalizedPath.lexically_relative(m_RootDirectory);

    // Prevent navigation outside Assets
    if (relativePath.empty() ||
        *relativePath.begin() == "..")
    {
        return false;
    }

    std::vector<FileSystem::FileEntry> entries;

    if (!m_FileSystem.ListDirectory(
        normalizedPath, entries))
    {
        return false;
    }

    m_CurrentDirectory = normalizedPath;
    m_Entries = std::move(entries);
    m_Selection.ClearContentSelection();

    return true;
}

bool WindowContentBrowser::CanNavigateUp() const
{
    return m_CurrentDirectory != m_RootDirectory;
}

void WindowContentBrowser::NavigateUp()
{
    if (!CanNavigateUp())
    {
        return;
    }

    NavigateTo(m_CurrentDirectory.parent_path());
}

void WindowContentBrowser::DrawNavigationBar()
{
    ImGui::BeginDisabled(!CanNavigateUp());

    if (ImGui::Button(EditorIcons::ArrowUp))
    {
        NavigateUp();
    }

    ImGui::EndDisabled();
    if (CanNavigateUp()) DrawFolderDropTarget(m_CurrentDirectory.parent_path());

    ImGui::SameLine();

    if (ImGui::Button(EditorIcons::Refresh))
    {
        QueueAction(FileAction::Refresh);
    }

    ImGui::SameLine();

    DrawBreadcrumbs();

    ImGui::Separator();
}

void WindowContentBrowser::DrawDirectoryContents()
{
    m_VisibleItemCount = 0;

    switch (m_ViewMode)
    {
    case ViewMode::Grid:
        DrawGridView();
        break;

    case ViewMode::List:
        DrawListView();
        break;
    }
}

void WindowContentBrowser::DrawGridView()
{
    std::filesystem::path pendingDirectory;

    const float availableWidth =
        ImGui::GetContentRegionAvail().x;

    const float tileWidth =
        m_ThumbnailSize + m_TilePadding * 2.0f;

    const float cellWidth =
        tileWidth + m_TileSpacing;

    const int columnCount = (std::max)(
        1,
        static_cast<int>(
            (availableWidth + m_TileSpacing) /
            cellWidth)
        );

    int column = 0;
    size_t visibleCount = 0;

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(m_TileSpacing, m_TileSpacing)
    );

    for (const auto& entry : m_Entries)
    {
        AssetID assetID = InvalidAssetID;

        if (!entry.IsDirectory)
        {
            assetID =
                m_AssetRegistry.GetAssetIDByPath(
                    entry.Path);

            if (assetID == InvalidAssetID)
            {
                continue;
            }

            if (!m_AssetRegistry.GetAsset(assetID))
            {
                continue;
            }
        }

        if (!MatchesFilter(entry, assetID)) continue;

        if (column > 0)
        {
            ImGui::SameLine();
        }

        if (DrawAssetTile(entry, assetID))
        {
            pendingDirectory = entry.Path;
        }

        visibleCount++;
        column++;

        if (column >= columnCount)
        {
            column = 0;
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
        m_Selection.ClearContentSelection();

    DrawBackgroundContextMenu();
    ImGui::PopStyleVar();
    m_VisibleItemCount = visibleCount;

    if (visibleCount == 0)
    {
        ImGui::TextDisabled("No matching assets or folders.");
    }

    if (!pendingDirectory.empty())
    {
        NavigateTo(pendingDirectory);
    }
}

void WindowContentBrowser::DrawFolderTree()
{
    ImGui::TextDisabled("FOLDERS");
    ImGui::Separator();

    DrawFolderNode(m_RootDirectory);
}

void WindowContentBrowser::DrawFolderNode(
    const std::filesystem::path& directory)
{
    if (!IsSafeDirectory(directory)) return;

    std::vector<FileSystem::FileEntry> entries;

    if (!m_FileSystem.ListDirectory(directory, entries))
    {
        return;
    }

    const bool hasChildren = std::any_of(
        entries.begin(),
        entries.end(),
        [](const FileSystem::FileEntry& entry)
        {
            return entry.IsDirectory;
        });

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (directory == m_CurrentDirectory)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!hasChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (directory == m_RootDirectory)
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    const std::string nodeID =
        directory.generic_string();

    const std::string name =
        std::string(
            directory == m_CurrentDirectory
            ? EditorIcons::FolderOpen
            : EditorIcons::Folder
        )
        + " " +
        directory.filename().string();

    bool opened = ImGui::TreeNodeEx(
        nodeID.c_str(),
        flags,
        "%s",
        name.c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsItemToggledOpen())
    {
        NavigateTo(directory);
    }

    FileSystem::FileEntry folderEntry;
    folderEntry.Path = directory;
    folderEntry.IsDirectory = true;
    DrawFolderDropTarget(directory);
    DrawEntryContextMenu(folderEntry, InvalidAssetID);

    if (opened)
    {
        for (const auto& entry : entries)
        {
            if (entry.IsDirectory)
            {
                DrawFolderNode(entry.Path);
            }
        }

        ImGui::TreePop();
    }
}

void WindowContentBrowser::DrawBreadcrumbs()
{
    const auto currentDirectory = m_CurrentDirectory;

    std::filesystem::path accumulatedPath;
    std::filesystem::path pendingDirectory;

    bool first = true;

    for (const auto& part : currentDirectory)
    {
        if (part.empty())
        {
            continue;
        }

        accumulatedPath /= part;

        if (!first)
        {
            ImGui::SameLine();
            ImGui::TextDisabled(">");
            ImGui::SameLine();
        }

        first = false;

        const std::string buttonID =
            accumulatedPath.generic_string();

        const std::string label = part.string();

        ImGui::PushID(buttonID.c_str());

        bool isCurrent =
            accumulatedPath == currentDirectory;

        ImGui::BeginDisabled(isCurrent);

        if (ImGui::Button(label.c_str()))
        {
            pendingDirectory = accumulatedPath;
        }

        ImGui::EndDisabled();

        ImGui::PopID();
    }

    if (!pendingDirectory.empty())
    {
        NavigateTo(pendingDirectory);
    }
}

bool WindowContentBrowser::DrawAssetTile(
    const FileSystem::FileEntry& entry,
    AssetID assetID)
{
    const bool isFolder = entry.IsDirectory;

    const AssetRecord* record = isFolder
        ? nullptr
        : m_AssetRegistry.GetAsset(assetID);

    if (!isFolder && !record)
    {
        return false;
    }

    const AssetVisual visual = isFolder
        ? AssetVisuals::GetFolder()
        : AssetVisuals::Get(record->Type);

    const std::string name = isFolder
        ? entry.Path.filename().string()
        : record->Name;

    const std::string tileID =
        entry.Path.generic_string();

    const bool selected =
        isFolder
        ? m_Selection.GetType() == EditorSelectionType::Folder &&
          m_Selection.GetFolder() == entry.Path.lexically_normal()
        : m_Selection.GetAssetID() == assetID;

    const float tileWidth =
        m_ThumbnailSize + m_TilePadding * 2.0f;

    const float textHeight =
        ImGui::GetTextLineHeight();

    const float tileHeight =
        m_ThumbnailSize +
        m_TilePadding * 3.0f +
        textHeight * 2.0f +
        6.0f;

    ImGui::PushID(tileID.c_str());

    // Interaction area
    ImGui::InvisibleButton(
        "##AssetTile",
        ImVec2(tileWidth, tileHeight)
    );

    const bool hovered = ImGui::IsItemHovered();

    const bool clicked =
        ImGui::IsItemClicked(ImGuiMouseButton_Left);

    const bool doubleClicked =
        hovered &&
        ImGui::IsMouseDoubleClicked(
            ImGuiMouseButton_Left);

    const ImVec2 tileMin = ImGui::GetItemRectMin();
    const ImVec2 tileMax = ImGui::GetItemRectMax();

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    // Tile background
    ImU32 backgroundColor =
        IM_COL32(34, 34, 40, 255);

    if (selected)
    {
        backgroundColor =
            IM_COL32(55, 35, 43, 255);
    }
    else if (hovered)
    {
        backgroundColor =
            IM_COL32(45, 45, 53, 255);
    }

    drawList->AddRectFilled(
        tileMin,
        tileMax,
        backgroundColor,
        5.0f
    );

    // Tile border
    ImU32 borderColor =
        IM_COL32(48, 48, 56, 255);

    if (selected)
    {
        borderColor =
            IM_COL32(198, 49, 70, 255);
    }
    else if (hovered)
    {
        borderColor =
            IM_COL32(85, 85, 95, 255);
    }

    drawList->AddRect(
        tileMin,
        tileMax,
        borderColor,
        5.0f
    );

    // Preview area
    const ImVec2 previewMin(
        tileMin.x + m_TilePadding,
        tileMin.y + m_TilePadding
    );

    const ImVec2 previewMax(
        tileMax.x - m_TilePadding,
        previewMin.y + m_ThumbnailSize
    );

    drawList->AddRectFilled(
        previewMin,
        previewMax,
        IM_COL32(16, 16, 19, 255),
        4.0f
    );

    // Asset icon
    const float iconSize =
        m_ThumbnailSize * 0.48f;

    ImGui::PushFont(nullptr, iconSize);

    const ImVec2 iconTextSize =
        ImGui::CalcTextSize(visual.Icon);

    const ImVec2 iconPosition(
        previewMin.x +
        (m_ThumbnailSize - iconTextSize.x) * 0.5f,

        previewMin.y +
        (m_ThumbnailSize - iconTextSize.y) * 0.5f
    );

    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFontSize(),
        iconPosition,
        visual.Color,
        visual.Icon
    );

    ImGui::PopFont();

    // Text area
    const ImVec2 namePosition(
        previewMin.x,
        previewMax.y + m_TilePadding
    );

    const ImVec2 typePosition(
        namePosition.x,
        namePosition.y + textHeight + 2.0f
    );

    // Prevent long names from overflowing
    drawList->PushClipRect(
        ImVec2(previewMin.x, previewMax.y),
        ImVec2(previewMax.x, tileMax.y - 4.0f),
        true
    );

    const float textWidth = previewMax.x - previewMin.x;

    const std::string displayName =
        TruncateText(name, textWidth);

    const std::string displayType =
        TruncateText(visual.Label, textWidth);

    drawList->AddText(
        namePosition,
        ImGui::GetColorU32(ImGuiCol_Text),
        displayName.c_str()
    );

    drawList->AddText(
        typePosition,
        visual.Color,
        displayType.c_str()
    );

    drawList->PopClipRect();

    // Asset type color indicator
    drawList->AddRectFilled(
        ImVec2(tileMin.x + 4.0f, tileMax.y - 3.0f),
        ImVec2(tileMax.x - 4.0f, tileMax.y - 1.0f),
        visual.Color,
        1.0f
    );

    // Selection
    if (clicked)
    {
        if (isFolder) m_Selection.SelectFolder(entry.Path);
        else m_Selection.SelectAsset(assetID);
    }

    // Tooltip
    const bool showTooltip = hovered && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal);
    if (isFolder) DrawFolderDropTarget(entry.Path);
    else DrawAssetDragSource(assetID);
    DrawEntryContextMenu(entry, assetID);
    if (showTooltip && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
    {
        ImGui::BeginTooltip();

        ImGui::TextUnformatted(name.c_str());
        ImGui::TextDisabled("%s", visual.Label);

        ImGui::Separator();

        ImGui::TextUnformatted(tileID.c_str());

        ImGui::EndTooltip();
    }

    ImGui::PopID();

    return isFolder && doubleClicked;
}

bool WindowContentBrowser::DrawAssetListItem(
    const FileSystem::FileEntry& entry,
    AssetID assetID)
{
    const bool isFolder = entry.IsDirectory;

    const AssetRecord* record = isFolder
        ? nullptr
        : m_AssetRegistry.GetAsset(assetID);

    if (!isFolder && !record)
    {
        return false;
    }

    const AssetVisual visual = isFolder
        ? AssetVisuals::GetFolder()
        : AssetVisuals::Get(record->Type);

    const std::string name = isFolder
        ? entry.Path.filename().string()
        : record->Name;

    const std::string itemID =
        entry.Path.generic_string();

    const bool selected =
        isFolder
        ? m_Selection.GetType() == EditorSelectionType::Folder &&
          m_Selection.GetFolder() == entry.Path.lexically_normal()
        : m_Selection.GetAssetID() == assetID;

    ImGui::PushID(itemID.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGui::TextColored(
        ImGui::ColorConvertU32ToFloat4(visual.Color),
        "%s",
        visual.Icon
    );

    ImGui::SameLine();

    const std::string label = name + "###AssetRow";

    bool clicked = ImGui::Selectable(
        label.c_str(),
        selected,
        ImGuiSelectableFlags_SpanAllColumns |
        ImGuiSelectableFlags_AllowDoubleClick
    );

    bool doubleClicked =
        ImGui::IsItemHovered() &&
        ImGui::IsMouseDoubleClicked(
            ImGuiMouseButton_Left);

    if (clicked)
    {
        if (isFolder) m_Selection.SelectFolder(entry.Path);
        else m_Selection.SelectAsset(assetID);
    }

    if (isFolder) DrawFolderDropTarget(entry.Path);
    else DrawAssetDragSource(assetID);
    DrawEntryContextMenu(entry, assetID);

    ImGui::TableSetColumnIndex(1);

    ImGui::TextColored(
        ImGui::ColorConvertU32ToFloat4(visual.Color),
        "%s",
        visual.Label
    );

    ImGui::TableSetColumnIndex(2);

    if (isFolder)
    {
        ImGui::TextDisabled("-");
    }
    else
    {
        ImGui::TextUnformatted(
            entry.Path.extension().string().c_str()
        );
    }

    ImGui::PopID();

    return isFolder && doubleClicked;
}

void WindowContentBrowser::DrawListView()
{
    std::filesystem::path pendingDirectory;

    ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable(
        "AssetListTable",
        3,
        flags))
    {
        ImGui::TableSetupColumn(
            "Name",
            ImGuiTableColumnFlags_WidthStretch,
            2.0f
        );

        ImGui::TableSetupColumn(
            "Type",
            ImGuiTableColumnFlags_WidthStretch,
            1.0f
        );

        ImGui::TableSetupColumn(
            "Extension",
            ImGuiTableColumnFlags_WidthFixed,
            90.0f
        );

        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableHeadersRow();

        for (const auto& entry : m_Entries)
        {
            AssetID assetID = InvalidAssetID;

            if (!entry.IsDirectory)
            {
                assetID =
                    m_AssetRegistry.GetAssetIDByPath(
                        entry.Path);

                if (assetID == InvalidAssetID ||
                    !m_AssetRegistry.GetAsset(assetID))
                {
                    continue;
                }
            }

            if (!MatchesFilter(entry, assetID)) continue;

            if (DrawAssetListItem(entry, assetID))
            {
                pendingDirectory = entry.Path;
            }

            m_VisibleItemCount++;
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
            m_Selection.ClearContentSelection();

        DrawBackgroundContextMenu();
        ImGui::EndTable();

    }

    if (!pendingDirectory.empty())
    {
        NavigateTo(pendingDirectory);
    }
}

void WindowContentBrowser::DrawStatusBar()
{
    ImGui::Separator();

    const bool hasSelection =
        m_Selection.GetType() == EditorSelectionType::Folder ||
        (m_Selection.GetType() == EditorSelectionType::Asset &&
         m_AssetRegistry.Contains(m_Selection.GetAssetID()));

    ImGui::TextDisabled(
        "%zu items | %d selected",
        m_VisibleItemCount,
        hasSelection ? 1 : 0
    );

    const float buttonWidth =
        ImGui::GetFrameHeight();

    const float sliderWidth = 100.0f;

    const float spacing =
        ImGui::GetStyle().ItemSpacing.x;

    const float controlsWidth =
        buttonWidth * 2.0f +
        sliderWidth +
        spacing * 2.0f;

    const float rightEdge =
        ImGui::GetWindowContentRegionMax().x;

    ImGui::SameLine();

    ImGui::SetCursorPosX(
        (std::max)(
            ImGui::GetCursorPosX(),
            rightEdge - controlsWidth
            )
    );

    // Grid view button
    if (m_ViewMode == ViewMode::Grid)
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImGui::GetStyleColorVec4(ImGuiCol_Header)
        );
    }

    const bool gridClicked = ImGui::Button(
        EditorIcons::Grid,
        ImVec2(buttonWidth, buttonWidth)
    );

    if (m_ViewMode == ViewMode::Grid)
    {
        ImGui::PopStyleColor();
    }

    if (gridClicked)
    {
        m_ViewMode = ViewMode::Grid;
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Grid View");
    }

    ImGui::SameLine();

    // List view button
    if (m_ViewMode == ViewMode::List)
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImGui::GetStyleColorVec4(ImGuiCol_Header)
        );
    }

    const bool listClicked = ImGui::Button(
        EditorIcons::List,
        ImVec2(buttonWidth, buttonWidth)
    );

    if (m_ViewMode == ViewMode::List)
    {
        ImGui::PopStyleColor();
    }

    if (listClicked)
    {
        m_ViewMode = ViewMode::List;
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("List View");
    }

    ImGui::SameLine();

    // Thumbnail size
    ImGui::BeginDisabled(
        m_ViewMode == ViewMode::List);

    ImGui::SetNextItemWidth(sliderWidth);

    ImGui::SliderFloat(
        "##ThumbnailSize",
        &m_ThumbnailSize,
        m_MinThumbnailSize,
        m_MaxThumbnailSize,
        "%.0f px"
    );

    ImGui::EndDisabled();
}

std::string WindowContentBrowser::TruncateText(
    const std::string& text,
    float maxWidth) const
{
    if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
    {
        return text;
    }

    const std::string ellipsis = "...";

    if (ImGui::CalcTextSize(ellipsis.c_str()).x > maxWidth)
    {
        return "";
    }

    std::string result = text;

    while (!result.empty())
    {
        // Remove one complete UTF-8 character
        size_t pos = result.size() - 1;

        while (pos > 0 &&
            (static_cast<unsigned char>(result[pos]) & 0xC0) == 0x80)
        {
            --pos;
        }

        result.resize(pos);

        const std::string candidate = result + ellipsis;

        if (ImGui::CalcTextSize(candidate.c_str()).x <= maxWidth)
        {
            return candidate;
        }
    }

    return ellipsis;
}

// File operations: requests are captured during drawing, then executed
// from the parent window after all entry iteration has finished.
void WindowContentBrowser::QueueAction(FileAction action, AssetID id,
                                       const std::filesystem::path& path)
{
    if (m_PendingAction != FileAction::None || m_ModalAction != FileAction::None)
        return;
    m_PendingAction = action;
    m_PendingID = id;
    m_PendingPath = path;
}

void WindowContentBrowser::ReportError(const std::string& message)
{
    m_StatusError = message;
}

bool WindowContentBrowser::IsSafeDirectory(const std::filesystem::path& path) const
{
    if (path.empty() || path.has_root_name() || path.has_root_directory()) return false;
    const auto normalized = path.lexically_normal();
    const auto relative = normalized.lexically_relative(m_RootDirectory);
    if (relative.empty() || *relative.begin() == "..") return false;

    // Do not traverse junctions or symbolic links beneath the project root.
    auto current = m_FileSystem.ResolvePath(m_RootDirectory);
    auto valid = [](const std::filesystem::path& p)
    {
        const DWORD attributes = GetFileAttributesW(p.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0;
    };
    if (!valid(current)) return false;
    for (const auto& part : relative)
    {
        if (part == "." || part.empty()) continue;
        current /= part;
        if (!valid(current)) return false;
    }
    return true;
}

bool WindowContentBrowser::IsValidFileName(const std::string& name) const
{
    if (name.empty() || name == "." || name == ".." ||
        name.find_first_of("<>:\"/\\|?*") != std::string::npos ||
        name.back() == '.' || name.back() == ' ') return false;
    for (unsigned char ch : name) if (ch < 32) return false;
    std::string stem = name.substr(0, name.find('.'));
    while (!stem.empty() && stem.back() == ' ') stem.pop_back();
    for (char& ch : stem) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL" ||
        stem == "CONIN$" || stem == "CONOUT$") return false;
    if (stem.size() == 4 && (stem.substr(0, 3) == "COM" || stem.substr(0, 3) == "LPT") &&
        stem[3] >= '0' && stem[3] <= '9') return false;
    return true;
}

bool WindowContentBrowser::CanModifyAsset(AssetID id, bool deleting) const
{
    const auto* record = m_AssetRegistry.GetAsset(id);
    if (!record || record->IsSubAsset()) return false;
    if (deleting)
    {
        for (AssetID related : m_AssetRegistry.GetAssetIDsByPath(record->SourcePath))
            if (m_ResourceManager.IsLoaded(related)) return false;
    }
    return true;
}

void WindowContentBrowser::DrawEntryContextMenu(const FileSystem::FileEntry& entry, AssetID id)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        if (entry.IsDirectory) m_Selection.SelectFolder(entry.Path);
        else m_Selection.SelectAsset(id);
    }
    const std::string popupID = "EntryContext##" + entry.Path.generic_string();
    if (!ImGui::BeginPopupContextItem(popupID.c_str())) return;
    if (entry.IsDirectory)
    {
        if (ImGui::MenuItem("Open")) QueueAction(FileAction::OpenFolder, InvalidAssetID, entry.Path);
        if (ImGui::MenuItem("New Folder")) QueueAction(FileAction::CreateFolder, InvalidAssetID, entry.Path);
    }
    else
    {
        if (ImGui::MenuItem("Rename", "F2", false, CanModifyAsset(id, false))) QueueAction(FileAction::Rename, id);
        if (ImGui::MenuItem("Move To...", nullptr, false, CanModifyAsset(id, false))) QueueAction(FileAction::Move, id);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Copy Path")) ImGui::SetClipboardText(entry.Path.generic_string().c_str());
    if (!entry.IsDirectory && ImGui::MenuItem("Copy Asset ID"))
        ImGui::SetClipboardText(std::to_string(id).c_str());
    if (ImGui::MenuItem("Show in Explorer")) ShowInExplorer(entry.Path, entry.IsDirectory);
    if (!entry.IsDirectory)
    {
        ImGui::Separator();
        if (ImGui::MenuItem("Delete", "Del", false, CanModifyAsset(id, true))) QueueAction(FileAction::Delete, id);
        if (!CanModifyAsset(id, true)) ImGui::TextDisabled("Deletion unavailable for cached resources or sub-assets.");
    }
    ImGui::EndPopup();
}

void WindowContentBrowser::DrawBackgroundContextMenu()
{
    if (!ImGui::BeginPopupContextWindow("DirectoryBackground",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) return;
    if (ImGui::MenuItem("New Folder", "Ctrl+Shift+N")) QueueAction(FileAction::CreateFolder, InvalidAssetID, m_CurrentDirectory);
    ImGui::Separator();
    if (ImGui::MenuItem("Refresh", "F5")) QueueAction(FileAction::Refresh);
    if (ImGui::MenuItem("Show in Explorer")) ShowInExplorer(m_CurrentDirectory, true);
    ImGui::EndPopup();
}

void WindowContentBrowser::HandleShortcuts()
{
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        ImGui::GetIO().WantTextInput || ImGui::IsAnyItemActive() ||
        ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) ||
        m_ModalAction != FileAction::None) return;
    const auto id = m_Selection.GetAssetID();
    // A shortcut must never affect an asset selected in another folder.
    const auto* record = m_AssetRegistry.GetAsset(id);
    FileSystem::FileEntry selectedEntry;
    if (record) selectedEntry.Path = record->SourcePath;
    const bool local = record && record->SourcePath.parent_path() == m_CurrentDirectory &&
        MatchesFilter(selectedEntry, id);
    if (ImGui::IsKeyPressed(ImGuiKey_F2, false) && local && CanModifyAsset(id, false)) QueueAction(FileAction::Rename, id);
    else if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) && local && CanModifyAsset(id, true)) QueueAction(FileAction::Delete, id);
    else if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) QueueAction(FileAction::Refresh);
    else if (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_N, false))
        QueueAction(FileAction::CreateFolder, InvalidAssetID, m_CurrentDirectory);
}

void WindowContentBrowser::ShowInExplorer(const std::filesystem::path& path, bool folder)
{
    const auto absolute = m_FileSystem.ResolvePath(path);
    if (!IsSafeDirectory(folder ? path : path.parent_path()) || !m_FileSystem.Exists(path))
    {
        ReportError("The requested path is unavailable.");
        return;
    }
    // Launch Explorer directly, not through cmd.exe.
    const std::wstring argument = folder ? L"\"" + absolute.wstring() + L"\""
        : L"/select,\"" + absolute.wstring() + L"\"";
    auto result = ShellExecuteW(nullptr, L"open", L"explorer.exe", argument.c_str(), nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) ReportError("Could not open Windows Explorer.");
}

void WindowContentBrowser::ProcessPendingAction()
{
    const auto action = m_PendingAction;
    if (action == FileAction::None) return;
    const auto id = m_PendingID;
    const auto path = m_PendingPath;
    m_PendingAction = FileAction::None;
    m_PendingID = InvalidAssetID;
    m_PendingPath.clear();
    if (action == FileAction::Refresh)
    {
        SynchronizeRegistry();
        if (!RefreshDirectory())
        {
            if (!NavigateTo(m_RootDirectory))
            {
                m_Entries.clear();
                m_Selection.ClearContentSelection();
                ReportError("Assets directory is unavailable.");
            }
        }
        return;
    }
    if (action == FileAction::DropMove)
    {
        if (!MoveAssetToFolder(id, path)) ReportError("Move failed. Check destination, metadata and engine log.");
        if (!RefreshDirectory()) ReportError("Could not refresh the directory after move.");
        return;
    }
    if (action == FileAction::OpenFolder)
    {
        if (!NavigateTo(path)) ReportError("Could not open the directory.");
        return;
    }
    m_OperationError.clear();
    m_OperationID = id;
    m_OperationDirectory = action == FileAction::CreateFolder ? path : m_CurrentDirectory;
    m_OperationName[0] = '\0';
    if (action == FileAction::CreateFolder)
    {
        if (!IsSafeDirectory(path)) { ReportError("Invalid destination directory."); return; }
        std::snprintf(m_OperationName, sizeof(m_OperationName), "%s", "New Folder");
    }
    else
    {
        const auto* record = m_AssetRegistry.GetAsset(id);
        if (!record || !CanModifyAsset(id, action == FileAction::Delete))
        { ReportError("The asset is unavailable for this operation."); return; }
        if (action == FileAction::Rename)
        {
            const auto name = PathToUTF8(record->SourcePath.stem());
            if (name.size() >= sizeof(m_OperationName)) { ReportError("Asset name is too long."); return; }
            std::snprintf(m_OperationName, sizeof(m_OperationName), "%s", name.c_str());
        }
    }
    m_ModalAction = action;
    ImGui::OpenPopup("Asset Operation");
}

void WindowContentBrowser::DrawOperationModal()
{
    if (m_ModalAction == FileAction::None) return;
    if (!ImGui::BeginPopupModal("Asset Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    const auto action = m_ModalAction;
    const auto* record = m_AssetRegistry.GetAsset(m_OperationID);
    const bool create = action == FileAction::CreateFolder;
    const bool rename = action == FileAction::Rename;
    const bool move = action == FileAction::Move;
    const bool deleting = action == FileAction::Delete;
    ImGui::TextUnformatted(create ? "Create Folder" : rename ? "Rename Asset" : move ? "Move Asset" : "Delete Asset");
    ImGui::Separator();
    if (record) ImGui::TextWrapped("Asset: %s", record->Name.c_str());
    if (create || rename)
    {
        ImGui::SetNextItemWidth(320.0f);
        ImGui::InputText("Name", m_OperationName, sizeof(m_OperationName));
        if (rename && record) ImGui::TextDisabled("Extension: %s", record->SourcePath.extension().string().c_str());
    }
    if (move || create)
        ImGui::TextWrapped("Destination: %s", m_OperationDirectory.generic_string().c_str());
    if (move)
    {
        if (ImGui::Button("Up") && m_OperationDirectory != m_RootDirectory)
            m_OperationDirectory = m_OperationDirectory.parent_path();
        std::vector<FileSystem::FileEntry> entries;
        const bool listed = IsSafeDirectory(m_OperationDirectory) && m_FileSystem.ListDirectory(m_OperationDirectory, entries);
        if (ImGui::BeginChild("MoveDirectories", ImVec2(340, 180), ImGuiChildFlags_Borders))
        {
            if (!listed) ImGui::TextDisabled("Directory unavailable.");
            else for (const auto& entry : entries)
            {
                if (!entry.IsDirectory || !IsSafeDirectory(entry.Path)) continue;
                ImGui::PushID(entry.Path.generic_string().c_str());
                if (ImGui::Selectable(entry.Path.filename().string().c_str())) m_OperationDirectory = entry.Path;
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
    if (deleting) ImGui::TextColored(ImVec4(0.93f, 0.28f, 0.36f, 1.0f), "This permanently deletes the source and metadata. No undo.");
    if (!m_OperationError.empty()) ImGui::TextColored(ImVec4(0.93f, 0.28f, 0.36f, 1.0f), "%s", m_OperationError.c_str());
    const bool valid = create ? IsValidFileName(m_OperationName) :
        record && CanModifyAsset(m_OperationID, deleting) && (!rename || IsValidFileName(m_OperationName));
    ImGui::BeginDisabled(!valid);
    const bool confirm = ImGui::Button(create ? "Create" : rename ? "Rename" : move ? "Move" : "Delete");
    ImGui::EndDisabled();
    ImGui::SameLine();
    const bool cancel = ImGui::Button("Cancel");
    bool close = cancel;
    if (confirm && valid)
    {
        bool success = false;
        m_OperationError.clear();
        try
        {
            if (create)
            {
                const auto dest = m_OperationDirectory / PathFromUTF8(m_OperationName);
                if (IsSafeDirectory(m_OperationDirectory) && !m_FileSystem.Exists(dest))
                {
                    // Atomic single-directory creation: never report an existing folder as newly created.
                    std::error_code ec;
                    success = std::filesystem::create_directory(m_FileSystem.ResolvePath(dest), ec) && !ec;
                    if (success && m_OperationDirectory == m_CurrentDirectory) m_Selection.SelectFolder(dest);
                }
            }
            else if (record && IsSafeDirectory(record->SourcePath.parent_path()))
            {
                const auto source = record->SourcePath;
                const auto metaPath = AssetMetadataSerializer::GetMetadataPath(source);
                const DWORD sourceAttributes = GetFileAttributesW(m_FileSystem.ResolvePath(source).c_str());
                const DWORD metaAttributes = GetFileAttributesW(m_FileSystem.ResolvePath(metaPath).c_str());
                AssetMetadata metadata;
                AssetMetadataSerializer serializer(m_FileSystem);
                const auto safeFile = [](DWORD a) { return a != INVALID_FILE_ATTRIBUTES && !(a & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)); };
                if (safeFile(sourceAttributes) && safeFile(metaAttributes) &&
                    serializer.LoadMetadata(source, metadata) && metadata.ID == m_OperationID && metadata.Type == record->Type)
                {
                    if (rename)
                    {
                        // Existing registry API accepts a narrow filename. Convert UTF-8 to the filesystem's narrow encoding.
                        success = m_AssetRegistry.RenameAsset(m_OperationID, PathFromUTF8(m_OperationName).string());
                    }
                    else if (move && IsSafeDirectory(m_OperationDirectory))
                        success = MoveAssetToFolder(m_OperationID, m_OperationDirectory);
                    else if (deleting)
                        success = m_AssetRegistry.DeleteAsset(m_OperationID);
                }
            }
        }
        catch (const std::exception& e) { m_OperationError = e.what(); }
        if (!create && !m_AssetRegistry.Contains(m_OperationID))
        {
            if (m_Selection.GetAssetID() == m_OperationID) m_Selection.ClearContentSelection();
        }
        else if (success && move && m_OperationDirectory != m_CurrentDirectory && m_Selection.GetAssetID() == m_OperationID)
            m_Selection.ClearContentSelection();
        const bool refreshed = RefreshDirectory();
        if (success)
        {
            close = true;
            if (!refreshed) ReportError("Operation completed, but directory refresh failed.");
        }
        else
        {
            if (m_OperationError.empty()) m_OperationError = "Operation failed. Check the name, destination, permissions and engine log. Disk changes may be partial.";
            if (!create && !m_AssetRegistry.Contains(m_OperationID))
            {
                ReportError(m_OperationError);
                close = true;
            }
        }
    }
    if (close)
    {
        m_ModalAction = FileAction::None;
        m_OperationID = InvalidAssetID;
        m_OperationDirectory.clear();
        m_OperationError.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}


void WindowContentBrowser::DrawSearchControls()
{
    ImGui::SetNextItemWidth((std::max)(80.0f, ImGui::GetContentRegionAvail().x * 0.55f));
    ImGui::InputTextWithHint("##AssetSearch", "Search current folder...", m_Search, sizeof(m_Search));
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) m_Search[0] = '\0';
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    const char* label = m_TypeFilter == AssetType::Unknown ? "All types" : AssetVisuals::Get(m_TypeFilter).Label;
    if (ImGui::BeginCombo("##AssetTypeFilter", label))
    {
        if (ImGui::Selectable("All types", m_TypeFilter == AssetType::Unknown)) m_TypeFilter = AssetType::Unknown;
        constexpr AssetType types[] = {AssetType::Texture, AssetType::Sprite, AssetType::Mesh, AssetType::Model,
            AssetType::AnimationClip, AssetType::AnimationGraph, AssetType::Audio, AssetType::Material,
            AssetType::Scene, AssetType::Prefab};
        for (auto type : types)
            if (ImGui::Selectable(AssetVisuals::Get(type).Label, m_TypeFilter == type)) m_TypeFilter = type;
        ImGui::EndCombo();
    }
    // Hidden selections must not be modified by keyboard shortcuts.
    if (m_Selection.GetType() == EditorSelectionType::Asset || m_Selection.GetType() == EditorSelectionType::Folder)
    {
        bool visible = false;
        for (const auto& entry : m_Entries)
        {
            const auto id = entry.IsDirectory ? InvalidAssetID : m_AssetRegistry.GetAssetIDByPath(entry.Path);
            const bool selected = entry.IsDirectory ? m_Selection.GetType() == EditorSelectionType::Folder &&
                m_Selection.GetFolder() == entry.Path.lexically_normal() : id != InvalidAssetID && id == m_Selection.GetAssetID();
            if (selected && MatchesFilter(entry, id)) { visible = true; break; }
        }
        if (!visible) m_Selection.ClearContentSelection();
    }
}

bool WindowContentBrowser::MatchesFilter(const FileSystem::FileEntry& entry, AssetID id) const
{
    if (!entry.IsDirectory)
    {
        const auto* record = m_AssetRegistry.GetAsset(id);
        if (!record || (m_TypeFilter != AssetType::Unknown && record->Type != m_TypeFilter)) return false;
    }
    auto lower = [](std::string text) {
        for (char& ch : text) if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch - 'A' + 'a');
        return text;
    };
    return lower(PathToUTF8(entry.Path.filename())).find(lower(m_Search)) != std::string::npos;
}

void WindowContentBrowser::DrawAssetDragSource(AssetID id)
{
    if (!CanModifyAsset(id, false)) return;
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("SHARDCORE_ASSET_ID", &id, sizeof(id));
        if (const auto* record = m_AssetRegistry.GetAsset(id)) ImGui::TextUnformatted(record->Name.c_str());
        ImGui::EndDragDropSource();
    }
}

void WindowContentBrowser::DrawFolderDropTarget(const std::filesystem::path& folder)
{
    if (!ImGui::BeginDragDropTarget()) return;
    if (const auto* payload = ImGui::AcceptDragDropPayload("SHARDCORE_ASSET_ID"))
    {
        if (payload->DataSize == sizeof(AssetID) && payload->IsDelivery())
        {
            AssetID id = InvalidAssetID;
            std::memcpy(&id, payload->Data, sizeof(id));
            const auto* record = m_AssetRegistry.GetAsset(id);
            if (record && record->SourcePath.parent_path() != folder)
                QueueAction(FileAction::DropMove, id, folder);
        }
    }
    ImGui::EndDragDropTarget();
}

bool WindowContentBrowser::MoveAssetToFolder(AssetID id, const std::filesystem::path& folder)
{
    try
    {
        const auto* record = m_AssetRegistry.GetAsset(id);
        if (!record || !CanModifyAsset(id, false) || !IsSafeDirectory(folder) ||
            !IsSafeDirectory(record->SourcePath.parent_path())) return false;
        const auto source = record->SourcePath;
        const auto meta = AssetMetadataSerializer::GetMetadataPath(source);
        for (const auto& path : {source, meta})
        {
            const DWORD attrs = GetFileAttributesW(m_FileSystem.ResolvePath(path).c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) return false;
        }
        AssetMetadata metadata;
        AssetMetadataSerializer serializer(m_FileSystem);
        if (!serializer.LoadMetadata(source, metadata) || metadata.ID != id || metadata.Type != record->Type) return false;
        if (!m_AssetRegistry.MoveAsset(id, folder / source.filename())) return false;
        if (m_Selection.GetAssetID() == id && folder != m_CurrentDirectory) m_Selection.ClearContentSelection();
        return true;
    }
    catch (const std::exception&) { return false; }
}

bool WindowContentBrowser::SynchronizeRegistry()
{
    // Explicit, synchronous refresh. Never rebuild the whole registry or evict GPU resources here.
    try
    {
        if (!IsSafeDirectory(m_RootDirectory)) { ReportError("Assets directory is unavailable."); return false; }
        const auto root = m_FileSystem.ResolvePath(m_RootDirectory);
        std::vector<std::filesystem::path> discovered;
        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(root, ec), end;
        while (!ec && it != end)
        {
            const DWORD attrs = GetFileAttributesW(it->path().c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES) { ReportError("Cannot inspect asset tree. Registry unchanged."); return false; }
            if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) it.disable_recursion_pending();
            else if (!(attrs & FILE_ATTRIBUTE_DIRECTORY))
            {
                auto path = it->path().lexically_relative(m_FileSystem.GetProjectRoot()).lexically_normal();
                if (AssetTypeUtils::DetectAssetType(path) != AssetType::Unknown) discovered.push_back(path);
            }
            it.increment(ec);
        }
        if (ec) { ReportError("Cannot scan asset tree. Registry unchanged."); return false; }
        std::sort(discovered.begin(), discovered.end());
        AssetMetadataSerializer serializer(m_FileSystem);
        const auto records = m_AssetRegistry.GetAllAssets();
        bool success = true;
        std::vector<std::filesystem::path> fresh;
        // Only remove records whose source is confirmed missing. Keep cached assets and their children.
        for (const auto& record : records)
        {
            if (record.IsSubAsset()) continue;
            const auto relative = record.SourcePath.lexically_relative(m_RootDirectory);
            if (relative.empty() || *relative.begin() == "..") continue;
            const DWORD attrs = GetFileAttributesW(m_FileSystem.ResolvePath(record.SourcePath).c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES)
            {
                if ((attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) || !IsSafeDirectory(record.SourcePath.parent_path())) { success = false; continue; }
                AssetMetadata metadata;
                const auto metaPath = AssetMetadataSerializer::GetMetadataPath(record.SourcePath);
                const DWORD ma = GetFileAttributesW(m_FileSystem.ResolvePath(metaPath).c_str());
                if (ma == INVALID_FILE_ATTRIBUTES || (ma & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
                    !serializer.LoadMetadata(record.SourcePath, metadata) || metadata.ID != record.ID || metadata.Type != record.Type) success = false;
                continue;
            }
            const DWORD error = GetLastError();
            if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) { success = false; continue; }
            const auto ids = m_AssetRegistry.GetAssetIDsByPath(record.SourcePath);
            bool cached = false;
            for (auto id : ids) if (m_ResourceManager.IsLoaded(id)) cached = true;
            if (cached) { success = false; continue; }
            for (auto id : ids) m_AssetRegistry.RemoveAsset(id);
        }
        // Metadata-bearing files first: preserve IDs on external moves with sidecars.
        for (const auto& path : discovered)
        {
            if (m_AssetRegistry.ContainsPath(path)) continue;
            const auto meta = AssetMetadataSerializer::GetMetadataPath(path);
            const DWORD attrs = GetFileAttributesW(m_FileSystem.ResolvePath(meta).c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES)
            {
                if (GetLastError() == ERROR_FILE_NOT_FOUND) fresh.push_back(path);
                else success = false;
            }
            else if ((attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
                m_AssetRegistry.LoadExistingAsset(path) == InvalidAssetID) success = false;
        }
        // Match startup scan policy: do not mint new IDs while metadata conflicts exist.
        if (success)
            for (const auto& path : fresh)
                if (m_AssetRegistry.RegisterNewAsset(path) == InvalidAssetID) success = false;
        if (m_Selection.GetType() == EditorSelectionType::Asset && !m_AssetRegistry.Contains(m_Selection.GetAssetID()))
            m_Selection.ClearContentSelection();
        if (m_Selection.GetType() == EditorSelectionType::Folder && !IsSafeDirectory(m_Selection.GetFolder()))
            m_Selection.ClearContentSelection();
        if (!success) ReportError("Refresh partially completed. Check metadata conflicts, missing cached assets and filesystem permissions. No metadata was overwritten.");
        return success;
    }
    catch (const std::exception& e) { ReportError(std::string("Refresh failed: ") + e.what()); return false; }
}
