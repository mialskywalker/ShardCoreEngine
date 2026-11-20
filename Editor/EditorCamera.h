#pragma once

#include "Camera.h"
#include <cstdint>

class EditorCamera
{
public:
	bool Init(uint32_t width, uint32_t height);
	void Update(float deltaTime, bool viewportHovered);

	Camera& GetCamera() { return m_Camera; }

private:
	Camera m_Camera;

    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    float m_MoveSpeed = 2.0f;
    float m_MouseSensitivity = 0.15f;

    int m_LastMouseX = 0;
    int m_LastMouseY = 0;
    int m_LastWheel = 0;

    bool m_WasRotating = false;
};