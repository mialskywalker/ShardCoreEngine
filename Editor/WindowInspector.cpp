#include "WindowInspector.h"
#include "EditorSelection.h"
#include "AssetRegistry.h"
#include "ResourceManager.h"
#include "FileSystem.h"
#include "AssetVisuals.h"
#include <system_error>
#include <cmath>
#include "imgui.h"

#include "Scene.h"
#include "GameObject.h"

#include "TransformComponent.h"
#include "MeshComponent.h"

#include <cstdio>

void WindowInspector::Draw(Scene* scene, const EditorSelection& selection,
    const FileSystem& fileSystem, const AssetRegistry& registry,
    const ResourceManager& resources)
{
    if (selection.GetType() != EditorSelectionType::GameObject)
    {
        m_LastSelectedUID = 0;
        m_LastScene = nullptr;
        m_RotationInitialized = false;
        m_EditingRotation = false;
        m_PendingRemoveComponent = ComponentType::None;
    }

    if (ImGui::Begin("Inspector"))
    {
        switch (selection.GetType())
        {
        case EditorSelectionType::GameObject:
            DrawGameObject(scene, selection.GetGameObjectUID());
            break;
        case EditorSelectionType::Asset:
            DrawAsset(selection.GetAssetID(), fileSystem, registry, resources);
            break;
        case EditorSelectionType::Folder:
            DrawFolder(selection.GetFolder(), fileSystem);
            break;
        default:
            ImGui::TextDisabled("No selection");
            break;
        }
    }
    ImGui::End();
}

void WindowInspector::DrawGameObject(Scene* scene, uint64_t selectedUID)
{

    if (scene && selectedUID != 0)
    {
        GameObject* go = scene->GetGameObject(selectedUID);

        if (go)
        {
            // Update name buffer when selection changes
            if (m_LastSelectedUID != selectedUID || m_LastScene != scene)
            {
                std::snprintf(m_NameBuffer, sizeof(m_NameBuffer), "%s", go->GetName().c_str());
                m_LastSelectedUID = selectedUID;
                m_LastScene = scene;
                m_RotationInitialized = false;
                m_EditingRotation = false;
                m_PendingRemoveComponent = ComponentType::None;
            }

            // Name
            ImGui::SetNextItemWidth(-100.0f);
            ImGui::InputText("##Name", m_NameBuffer, sizeof(m_NameBuffer));

            ImGui::SameLine();

            if (ImGui::Button("Rename") && m_NameBuffer[0] != '\0')
            {
                go->SetName(m_NameBuffer);
            }

            // Active
            bool active = go->IsActive();
            if (ImGui::Checkbox("Active", &active))
            {
                go->SetActive(active);
            }

            // UID
            ImGui::TextDisabled("UID: %llu", static_cast<unsigned long long>(go->GetUID()));

            // Components
            ImGui::Separator();
            DrawComponents(go);

            // Add Component
            ImGui::Spacing();
            DrawAddComponent(go);

            // Remove Component
            if (m_PendingRemoveComponent != ComponentType::None)
            {
                go->RemoveComponent(m_PendingRemoveComponent);
                m_PendingRemoveComponent = ComponentType::None;
            }
        }
    }

}

void WindowInspector::DrawComponents(GameObject* go)
{
    if (auto* transform = go->GetTransform())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawTransformComponent(transform);
        }
    }

    auto* mesh = static_cast<MeshComponent*>(go->GetComponent(ComponentType::Mesh));
    if (mesh)
    {
        bool meshOpen = ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen);
        if (ImGui::BeginPopupContextItem("MeshContext"))
        {
            if (ImGui::MenuItem("Remove Component"))
            {
                m_PendingRemoveComponent = ComponentType::Mesh;
            }

            ImGui::EndPopup();
        }

        /*if (meshOpen)
        {
            DrawMeshComponent(mesh, meshLibrary);
        }*/
    }
}

void WindowInspector::DrawAddComponent(GameObject* go)
{
    if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f)))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        bool hasMesh =
            go->GetComponent(ComponentType::Mesh) != nullptr;

        if (ImGui::MenuItem("Mesh", nullptr, false, !hasMesh))
        {
            go->AddComponent(ComponentType::Mesh);
        }

        ImGui::EndPopup();
    }
}

void WindowInspector::DrawTransformComponent(TransformComponent* transform)
{
    if (!transform)
        return;

    // Position
    Vector3 position = transform->GetPosition();

    float pos[3] = { position.x, position.y, position.z };

    if (ImGui::DragFloat3("Position", pos, 0.1f))
    {
        transform->SetPosition({ pos[0], pos[1], pos[2] });
    }

    // Scale
    Vector3 scale = transform->GetScale();

    float scl[3] = { scale.x, scale.y, scale.z };

    if (ImGui::DragFloat3("Scale", scl, 0.1f))
    {
        transform->SetScale({ scl[0], scl[1], scl[2] });
    }

    // Rotation (Quaternion -> Euler angles)
    Quaternion rotation = transform->GetRotation();
    Vector3 euler = rotation.ToEuler();

    float rot[3] =
    {
        DirectX::XMConvertToDegrees(euler.x),
        DirectX::XMConvertToDegrees(euler.y),
        DirectX::XMConvertToDegrees(euler.z)
    };

    Quaternion currentRotation = transform->GetRotation();

    bool rotationChanged =
        std::abs(currentRotation.x - m_LastRotation.x) > 0.00001f ||
        std::abs(currentRotation.y - m_LastRotation.y) > 0.00001f ||
        std::abs(currentRotation.z - m_LastRotation.z) > 0.00001f ||
        std::abs(currentRotation.w - m_LastRotation.w) > 0.00001f;

    // Synchronize external rotation changes
    if (!m_EditingRotation &&
        (!m_RotationInitialized || rotationChanged))
    {
        Vector3 euler = currentRotation.ToEuler();

        m_RotationDegrees[0] = DirectX::XMConvertToDegrees(euler.x);
        m_RotationDegrees[1] = DirectX::XMConvertToDegrees(euler.y);
        m_RotationDegrees[2] = DirectX::XMConvertToDegrees(euler.z);

        m_LastRotation = currentRotation;
        m_RotationInitialized = true;
    }

    if (ImGui::DragFloat3("Rotation", m_RotationDegrees, 0.5f))
    {
        float pitch = DirectX::XMConvertToRadians(m_RotationDegrees[0]);
        float yaw = DirectX::XMConvertToRadians(m_RotationDegrees[1]);
        float roll = DirectX::XMConvertToRadians(m_RotationDegrees[2]);

        Quaternion rotation =
            Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);

        transform->SetRotation(rotation);

        m_LastRotation = transform->GetRotation();
    }

    // Must be immediately after DragFloat3
    m_EditingRotation = ImGui::IsItemActive();

    if (ImGui::Button("Reset Transform"))
    {
        transform->SetPosition(Vector3::Zero);
        transform->SetRotation(Quaternion::Identity);
        transform->SetScale(Vector3::One);

        // Reset Inspector Euler values
        m_RotationDegrees[0] = 0.0f;
        m_RotationDegrees[1] = 0.0f;
        m_RotationDegrees[2] = 0.0f;
    }
}

//void WindowInspector::DrawMeshComponent(MeshComponent* mesh, MeshLibrary* meshLibrary)
//{
//    if (!mesh || !meshLibrary)
//    {
//        return;
//    }
//
//    struct MeshOption
//    {
//        const char* name;
//        PrimitiveMeshType type;
//    };
//
//    const MeshOption options[] =
//    {
//        { "Triangle", PrimitiveMeshType::Triangle },
//        { "Quad",     PrimitiveMeshType::Quad },
//        { "Cube",     PrimitiveMeshType::Cube }
//    };
//
//    // Determine currently selected mesh
//    const char* selectedName = "None";
//    Mesh* currentMesh = mesh->GetMesh();
//
//    for (const auto& option : options)
//    {
//        if (currentMesh == meshLibrary->GetPrimitive(option.type).get() && currentMesh != nullptr)
//        {
//            selectedName = option.name;
//            break;
//        }
//    }
//
//    // Mesh selection dropdown
//    if (ImGui::BeginCombo("Primitive Mesh", selectedName))
//    {
//        // None option
//        if (ImGui::Selectable("None", currentMesh == nullptr))
//        {
//            mesh->SetMesh(nullptr);
//        }
//
//        // Available primitives
//        for (const auto& option : options)
//        {
//            auto primitive = meshLibrary->GetPrimitive(option.type);
//
//            bool isSelected = currentMesh == primitive.get() && primitive != nullptr;
//
//            if (ImGui::Selectable(option.name, isSelected))
//            {
//                mesh->SetMesh(primitive);
//            }
//
//            if (isSelected)
//            {
//                ImGui::SetItemDefaultFocus();
//            }
//        }
//
//        ImGui::EndCombo();
//    }
//}


namespace
{
    void Property(const char* label, const std::string& value)
    {
        ImGui::TextDisabled("%s", label);
        ImGui::TextWrapped("%s", value.c_str());
    }
}

void WindowInspector::DrawAsset(AssetID id, const FileSystem& fileSystem,
    const AssetRegistry& registry, const ResourceManager& resources)
{
    const AssetRecord* record = registry.GetAsset(id);
    if (!record)
    {
        ImGui::TextDisabled("Asset not found");
        return;
    }
    const auto visual = AssetVisuals::Get(record->Type);
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(visual.Color),
                       "%s %s", visual.Icon, visual.Label);
    ImGui::Separator();
    Property("Name", record->Name);
    Property("Asset ID", std::to_string(record->ID));
    if (ImGui::Button("Copy Asset ID"))
        ImGui::SetClipboardText(std::to_string(record->ID).c_str());
    Property("Source path", record->SourcePath.generic_string());
    Property("Imported path", record->ImportedPath.empty() ?
             "Not imported" : record->ImportedPath.generic_string());
    Property("Parent asset", record->ParentAssetID == InvalidAssetID ?
             "None" : std::to_string(record->ParentAssetID));
    Property("Extension", record->SourcePath.extension().string());
    std::error_code ec;
    const auto bytes = std::filesystem::file_size(
        fileSystem.ResolvePath(record->SourcePath), ec);
    Property("Source file size", ec ? "Unavailable" :
             std::to_string(bytes) + " bytes");
    Property("Resource status", resources.IsLoaded(id) ?
             "Cached" : "Not cached");
}

void WindowInspector::DrawFolder(const std::filesystem::path& path,
    const FileSystem& fileSystem)
{
    const auto visual = AssetVisuals::GetFolder();
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(visual.Color),
                       "%s Folder", visual.Icon);
    ImGui::Separator();
    Property("Name", path.filename().string());
    Property("Path", path.generic_string());
    std::error_code ec;
    const bool exists = std::filesystem::is_directory(fileSystem.ResolvePath(path), ec);
    Property("Status", exists && !ec ? "Available" : "Unavailable");
}
