#pragma once

#include "ComponentType.h"
#include <cstdint>
#include <filesystem>
#include "AssetTypes.h"
#include <SimpleMath.h>

class EditorSelection;
class AssetRegistry;
class ResourceManager;
class FileSystem;
class Scene;
class GameObject;
class TransformComponent;
class MeshComponent;

using DirectX::SimpleMath::Quaternion;

class WindowInspector
{
public:
    void Draw(Scene* scene, const EditorSelection& selection,
              const FileSystem& fileSystem, const AssetRegistry& registry,
              const ResourceManager& resources);

private:
    void DrawGameObject(Scene* scene, uint64_t selectedUID);
    void DrawAsset(AssetID id, const FileSystem& fileSystem,
                   const AssetRegistry& registry, const ResourceManager& resources);
    void DrawFolder(const std::filesystem::path& path, const FileSystem& fileSystem);
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