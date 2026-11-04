#pragma once

#include "Application.h"
#include "EditorUI.h"

class EditorApplication : public Application
{
public:
    bool Init();
    ~EditorApplication() override;

protected:
    void OnRenderOverlay() override;

private:
    EditorUI m_EditorUI;
};