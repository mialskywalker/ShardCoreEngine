#include "WindowViewport.h"
#include "RenderTexture.h"

#include <imgui.h>

void WindowViewport::Draw(RenderTexture* renderTexture)
{
    ImGui::Begin("Scene");

    if (renderTexture)
    {
        ImVec2 availableSize = ImGui::GetContentRegionAvail();

        if (availableSize.x > 0 && availableSize.y > 0)
        {
            ImTextureID textureID =
                (ImTextureID)renderTexture->GetSRV().ptr;

            ImGui::Image(textureID, availableSize);
        }
    }

    ImGui::End();
}