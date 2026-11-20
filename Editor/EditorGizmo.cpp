#include "EditorGizmo.h"

#include "Camera.h"
#include "GameObject.h"
#include "TransformComponent.h"

#include <imgui.h>
#include "ImGuizmo.h"

#include <algorithm>

void EditorGizmo::Draw(Camera& camera, GameObject* selectedObject, float viewportX, float viewportY, float viewportWidth, float viewportHeight, bool viewportHovered)
{
	if (!selectedObject || viewportWidth <= 0.0f || viewportHeight <= 0.0f)
	{
		return;
	}

	// Gizmo shortcuts
	if (viewportHovered && !ImGuizmo::IsUsing() && !ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::GetIO().WantTextInput)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_W)) m_Operation = GizmoOperation::Translate;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) m_Operation = GizmoOperation::Rotate;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) m_Operation = GizmoOperation::Scale;
	}

	// Select ImGuizmo operation
	ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;

	switch (m_Operation)
	{
	case GizmoOperation::Translate:
        operation = ImGuizmo::TRANSLATE;
		break;
	case GizmoOperation::Rotate:
        operation = ImGuizmo::ROTATE;
		break;
	case GizmoOperation::Scale:
        operation = ImGuizmo::SCALE;
		break;
	}

    Matrix view = camera.GetView();
    Matrix projection = camera.GetProjection();

    auto* transform = selectedObject->GetTransform();

    Matrix worldMatrix = transform->GetWorldMatrix();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight
    );

    // Scale uses Local mode
    ImGuizmo::MODE mode =
        (m_Operation == GizmoOperation::Scale)
        ? ImGuizmo::LOCAL
        : ImGuizmo::WORLD;

    ImGuizmo::Manipulate(
        &view._11,
        &projection._11,
        operation,
        mode,
        &worldMatrix._11
    );

    if (ImGuizmo::IsUsing())
    {
        Matrix localMatrix = worldMatrix;

        // Convert World to Local
        if (GameObject* parent = selectedObject->GetParent())
        {
            Matrix parentWorld =
                parent->GetTransform()->GetWorldMatrix();

            localMatrix = worldMatrix * parentWorld.Invert();
        }

        Vector3 position;
        Vector3 scale;
        Quaternion rotation;

        if (localMatrix.Decompose(scale, rotation, position))
        {
            switch (m_Operation)
            {
            case GizmoOperation::Translate:
                transform->SetPosition(position);
                break;

            case GizmoOperation::Rotate:
                transform->SetRotation(rotation);
                break;

            case GizmoOperation::Scale:
                constexpr float MIN_SCALE = 0.00001f;

                scale.x = (std::max)(scale.x, MIN_SCALE);
                scale.y = (std::max)(scale.y, MIN_SCALE);
                scale.z = (std::max)(scale.z, MIN_SCALE);

                transform->SetScale(scale);
                break;
            }
        }
    }
}