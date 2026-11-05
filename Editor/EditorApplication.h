#pragma once

#include "Application.h"
#include "EditorUI.h"

class EditorApplication : public Application
{
public:
    bool Init();
    ~EditorApplication() override; // we call this after rendering the scene and before PostRender

protected:
    void OnBeginFrame() override;
    void OnRenderOverlay() override;
    bool CanUpdateCamera() const override;

private:
    EditorUI m_EditorUI;
};