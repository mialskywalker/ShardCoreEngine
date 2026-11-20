#pragma once

#include <cstdint>

class RenderTexture;
class EditorGizmo;
class Camera;
class GameObject;

class WindowViewport
{
public:
    void Draw(RenderTexture* renderTexture, EditorGizmo& gizmo, Camera& camera, GameObject* selectedObject);

    bool IsHovered() const { return m_Hovered; }

public:
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

    float GetViewportX() const { return m_ViewportX; }
    float GetViewportY() const { return m_ViewportY; }

private:
    bool m_Hovered = false;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    float m_ViewportX = 0.0f;
    float m_ViewportY = 0.0f;
};