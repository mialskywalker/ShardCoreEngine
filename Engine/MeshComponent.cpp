#include "MeshComponent.h"
#include "GameObject.h"
#include <utility>

MeshComponent::MeshComponent(GameObject* owner)
	: Component(owner, ComponentType::Mesh)
{
}

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
	m_Mesh = std::move(mesh);
}

Mesh* MeshComponent::GetMesh() const
{
	return m_Mesh.get();
}

