#include "WindowInspector.h"
#include "imgui.h"

#include "Scene.h"
#include "GameObject.h"
#include "MeshLibrary.h"

#include "TransformComponent.h"
#include "MeshComponent.h"

#include <cstdio>
#include <SimpleMath.h>

void WindowInspector::Draw(Scene* scene, uint64_t selectedUID, MeshLibrary* meshLibrary)
{
	ImGui::Begin("Inspector");

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
            DrawComponents(go, meshLibrary);

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

	ImGui::End();
}

void WindowInspector::DrawComponents(GameObject* go, MeshLibrary* meshLibrary)
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

        if (meshOpen)
        {
            DrawMeshComponent(mesh, meshLibrary);
        }
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

    if (ImGui::DragFloat3("Rotation", m_RotationDegrees, 0.5f))
    {
        float pitch = DirectX::XMConvertToRadians(m_RotationDegrees[0]);
        float yaw = DirectX::XMConvertToRadians(m_RotationDegrees[1]);
        float roll = DirectX::XMConvertToRadians(m_RotationDegrees[2]);

        Quaternion rotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);

        transform->SetRotation(rotation);
    }

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

void WindowInspector::DrawMeshComponent(MeshComponent* mesh, MeshLibrary* meshLibrary)
{
    if (!mesh || !meshLibrary)
    {
        return;
    }

    struct MeshOption
    {
        const char* name;
        PrimitiveMeshType type;
    };

    const MeshOption options[] =
    {
        { "Triangle", PrimitiveMeshType::Triangle },
        { "Quad",     PrimitiveMeshType::Quad },
        { "Cube",     PrimitiveMeshType::Cube }
    };

    // Determine currently selected mesh
    const char* selectedName = "None";
    Mesh* currentMesh = mesh->GetMesh();

    for (const auto& option : options)
    {
        if (currentMesh == meshLibrary->GetPrimitive(option.type).get() && currentMesh != nullptr)
        {
            selectedName = option.name;
            break;
        }
    }

    // Mesh selection dropdown
    if (ImGui::BeginCombo("Primitive Mesh", selectedName))
    {
        // None option
        if (ImGui::Selectable("None", currentMesh == nullptr))
        {
            mesh->SetMesh(nullptr);
        }

        // Available primitives
        for (const auto& option : options)
        {
            auto primitive = meshLibrary->GetPrimitive(option.type);

            bool isSelected = currentMesh == primitive.get() && primitive != nullptr;

            if (ImGui::Selectable(option.name, isSelected))
            {
                mesh->SetMesh(primitive);
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}
