#pragma once

#include "MeshData.h"

class MeshPrimitives
{
public:
	static MeshData CreateTriangle();
	static MeshData CreateQuad();
	static MeshData CreateCube();
};