#include "WindowViewport.h"
#include "RenderTexture.h"
#include "EditorGizmo.h"
#include "Camera.h"
#include "GameObject.h"

#include <imgui.h>

void WindowViewport::Draw(RenderTexture* renderTexture, EditorGizmo& gizmo, Camera& camera, GameObject* selectedObject)
{
    m_Hovered = false;
    m_Width = 0;
    m_Height = 0;

    if (ImGui::Begin("Scene"))
    {
        ImVec2 availableSize = ImGui::GetContentRegionAvail();

        if (availableSize.x >= 1.0f && availableSize.y >= 1.0f)
        {
            m_Width = static_cast<uint32_t>(availableSize.x);
            m_Height = static_cast<uint32_t>(availableSize.y);

            if (renderTexture)
            {
                ImTextureID textureID = (ImTextureID)renderTexture->GetSRV().ptr;
                ImGui::Image(textureID, availableSize);

                // Get image position and screen coordinates
                ImVec2 imagePosition = ImGui::GetItemRectMin();
                m_ViewportX = imagePosition.x;
                m_ViewportY = imagePosition.y;

                m_Hovered = ImGui::IsItemHovered();

                gizmo.Draw(camera, selectedObject, m_ViewportX, m_ViewportY, availableSize.x, availableSize.y, m_Hovered);
            }
        }
    }

    ImGui::End();
}