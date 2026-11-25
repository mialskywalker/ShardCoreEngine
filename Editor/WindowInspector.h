#pragma once

#include "ComponentType.h"
#include <cstdint>
#include <SimpleMath.h>

class Scene;
class GameObject;
class TransformComponent;
class MeshComponent;

using DirectX::SimpleMath::Quaternion;

class WindowInspector
{
public:
    void Draw(Scene* scene, uint64_t selectedUID);

private:
    void DrawComponents(GameObject* go);
    void DrawAddComponent(GameObject* go);
    void DrawTransformComponent(TransformComponent* transform);
    //void DrawMeshComponent(MeshComponent* mesh, MeshLibrary* meshLibrary);

private:
    char m_NameBuffer[128] = {};
    float m_RotationDegrees[3] = {};
    uint64_t m_LastSelectedUID = 0;
    Scene* m_LastScene = nullptr;
    ComponentType m_PendingRemoveComponent = ComponentType::None;

    Quaternion m_LastRotation = Quaternion::Identity;
    bool m_RotationInitialized = false;
    bool m_EditingRotation = false;
};