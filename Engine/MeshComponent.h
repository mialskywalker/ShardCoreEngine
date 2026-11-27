#pragma once

#include "Component.h"
#include "AssetReference.h"
#include <memory>

class ResourceManager;
class Mesh;

class MeshComponent : public Component
{
public:
	MeshComponent(GameObject* owner);

	void SetMesh(std::shared_ptr<Mesh> mesh);
	Mesh* GetMesh() const;

	bool SetMeshAsset(AssetID id, ResourceManager& resourceManager);
	AssetID GetMeshAssetID() const;

	void ClearMesh();

private:
	std::shared_ptr<Mesh> m_Mesh;
	AssetReference m_MeshAsset;
};