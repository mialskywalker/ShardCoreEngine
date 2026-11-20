#include "MeshLibrary.h"

#include "ResourceModule.h"
#include "MeshData.h"
#include "MeshPrimitives.h"
#include "Mesh.h"

bool MeshLibrary::Init(ResourceModule& resourceModule)
{
	// Triangle
	MeshData triangleData = MeshPrimitives::CreateTriangle();
	auto triangleMesh = std::make_shared<Mesh>();

	if (!triangleMesh->Init(resourceModule, triangleData.vertices, triangleData.indices))
	{
		return false;
	}

	m_Primitives[PrimitiveMeshType::Triangle] = std::move(triangleMesh);

	// Quad
	MeshData quadData = MeshPrimitives::CreateQuad();
	auto quadMesh = std::make_shared<Mesh>();

	if (!quadMesh->Init(resourceModule, quadData.vertices, quadData.indices))
	{
		return false;
	}

	m_Primitives[PrimitiveMeshType::Quad] = std::move(quadMesh);

	// Cube
	MeshData cubeData = MeshPrimitives::CreateCube();
	auto cubeMesh = std::make_shared<Mesh>();

	if (!cubeMesh->Init(resourceModule, cubeData.vertices, cubeData.indices))
	{
		return false;
	}

	m_Primitives[PrimitiveMeshType::Cube] = std::move(cubeMesh);

	return true;
}

std::shared_ptr<Mesh> MeshLibrary::GetPrimitive(PrimitiveMeshType type) const
{
	auto it = m_Primitives.find(type);
	if (it == m_Primitives.end())
	{
		return nullptr;
	}

	return it->second;
}
