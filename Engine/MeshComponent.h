#pragma once

#include "Component.h"
#include <memory>

class Mesh;

class MeshComponent : public Component
{
public:
	MeshComponent(GameObject* owner);

	void SetMesh(std::shared_ptr<Mesh> mesh);
	Mesh* GetMesh() const;

private:
	std::shared_ptr<Mesh> m_Mesh;
};