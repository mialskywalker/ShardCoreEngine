#include "MeshComponent.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "GameObject.h"
#include <utility>

MeshComponent::MeshComponent(GameObject* owner)
	: Component(owner, ComponentType::Mesh)
{
}

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
    m_Mesh = std::move(mesh);
    m_MeshAsset.Reset();
}

Mesh* MeshComponent::GetMesh() const
{
	return m_Mesh.get();
}

bool MeshComponent::SetMeshAsset(AssetID id, ResourceManager& resourceManager)
{
    if (id == InvalidAssetID)
    {
        ClearMesh();
        return true;
    }

    auto mesh = resourceManager.Load<Mesh>(id);

    if (!mesh)
    {
        return false;
    }

    m_Mesh = std::move(mesh);
    m_MeshAsset.ID = id;

    return true;
}

AssetID MeshComponent::GetMeshAssetID() const
{
    return m_MeshAsset.ID;
}

void MeshComponent::ClearMesh()
{
    m_Mesh.reset();
    m_MeshAsset.Reset();
}

