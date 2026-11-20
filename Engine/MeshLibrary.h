#pragma once
#include <memory>
#include <unordered_map>

enum class PrimitiveMeshType
{
	Triangle,
	Quad,
	Cube
};

class ResourceModule;
class Mesh;

class MeshLibrary
{
public:
	bool Init(ResourceModule& resourceModule);

	std::shared_ptr<Mesh> GetPrimitive(PrimitiveMeshType type) const;

private:
	std::unordered_map<PrimitiveMeshType, std::shared_ptr<Mesh>> m_Primitives;
};