#include "EditorCamera.h"
#include "Input.h"

#include <algorithm>
#include <DirectXMath.h>

bool EditorCamera::Init(uint32_t width, uint32_t height)
{
    return m_Camera.Init(width, height);
}

void EditorCamera::Update(float deltaTime, bool viewportHovered)
{
    int mouseX = Input::GetMouseDeltaX();
    int mouseY = Input::GetMouseDeltaY();
    int mouseWheel = Input::GetMouseWheelValue();

    // Camera input is only allowed inside Scene Viewport
    if (!viewportHovered)
    {
        m_WasRotating = false;
        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;
        m_LastWheel = mouseWheel;
        return;
    }

    bool rotating = Input::IsMouseButtonDown(MouseButton::Right);

    if (rotating)
    {
        // Ignore mouse movement on the first RMB frame
        if (m_WasRotating)
        {
            float deltaX = float(m_LastMouseX - mouseX);
            float deltaY = float(m_LastMouseY - mouseY);

            m_Yaw += DirectX::XMConvertToRadians(deltaX * m_MouseSensitivity);
            m_Pitch += DirectX::XMConvertToRadians(deltaY * m_MouseSensitivity);

            // Prevent camera from rotating upside down
            m_Pitch = std::clamp(m_Pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);

            Quaternion rotation = Quaternion::CreateFromYawPitchRoll(m_Yaw, m_Pitch, 0.0f);

            m_Camera.SetRotation(rotation);
        }

        // Movement speed
        float speed = m_MoveSpeed;

        if (Input::IsKeyDown(KeyCode::LeftShift)) speed *= 3.0f;

        Vector3 position = m_Camera.GetPosition();
        Quaternion rotation = m_Camera.GetRotation();

        Vector3 forward = Vector3::Transform(Vector3(0, 0, -1), rotation);
        Vector3 right = Vector3::Transform(Vector3(1, 0, 0), rotation);
        Vector3 up = Vector3::Transform(Vector3(0, 1, 0), rotation);

        float movement = speed * deltaTime;

        if (Input::IsKeyDown(KeyCode::W)) position += forward * movement;
        if (Input::IsKeyDown(KeyCode::S)) position -= forward * movement;
        if (Input::IsKeyDown(KeyCode::A)) position -= right * movement;
        if (Input::IsKeyDown(KeyCode::D)) position += right * movement;
        if (Input::IsKeyDown(KeyCode::Q)) position -= up * movement;
        if (Input::IsKeyDown(KeyCode::E)) position += up * movement;

        m_Camera.SetPosition(position);
    }

    m_WasRotating = rotating;

    // Mouse scroll zoom
    int wheelDelta = mouseWheel - m_LastWheel;

    if (wheelDelta != 0)
    {
        float scrollSteps = float(wheelDelta) / 120.0f;

        Vector3 forward = Vector3::Transform(Vector3(0, 0, -1), m_Camera.GetRotation());
        Vector3 position = m_Camera.GetPosition();

        position += forward * scrollSteps;

        m_Camera.SetPosition(position);
    }

    // Save mouse state for next frame
    m_LastMouseX = mouseX;
    m_LastMouseY = mouseY;
    m_LastWheel = mouseWheel;
}
