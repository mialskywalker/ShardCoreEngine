#pragma once

#include "Application.h"
#include "EditorUI.h"
#include "EditorCamera.h"

class EditorApplication : public Application
{
public:
    bool Init();
    ~EditorApplication() override; // we call this after rendering the scene and before PostRender

protected:
    void OnBeginFrame() override;
    void OnRenderOverlay() override;
    void OnUpdate(float deltaTime) override;
    Camera& GetRenderCamera() override;

private:
    EditorUI m_EditorUI;
    EditorCamera m_EditorCamera;
};