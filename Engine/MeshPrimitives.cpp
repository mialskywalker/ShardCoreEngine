#include "MeshPrimitives.h"

MeshData MeshPrimitives::CreateTriangle()
{
	MeshData data;

    data.vertices =
	{
		{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	};

	data.indices = {
        0, 1, 2
    };

	return data;
}

MeshData MeshPrimitives::CreateQuad()
{
	MeshData data;

    data.vertices =
	{
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom Left
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top Left
		{{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top Right
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}}  // Bottom Right
	};

    data.indices =
	{
		0, 1, 2,
		0, 2, 3
	};

	return data;
}

MeshData MeshPrimitives::CreateCube()
{
	MeshData data;

    // 8 vertices
    data.vertices =
    {
        // Back face (Z = -1)
        {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}}, // 0
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // 1
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}}, // 2
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}}, // 3

        // Front face (Z = 1)
        {{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f}}, // 4
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f}}, // 5
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f, 1.0f}}, // 6
        {{ 1.0f, -1.0f,  1.0f}, {0.5f, 0.5f, 0.5f}}  // 7
    };

    // 36 indices (12 triangles)
    data.indices =
    {
        // Back (-Z)
        0, 1, 2,
        0, 2, 3,

        // Front (+Z)
        4, 7, 6,
        4, 6, 5,

        // Left (-X)
        0, 4, 5,
        0, 5, 1,

        // Right (+X)
        3, 2, 6,
        3, 6, 7,

        // Top (+Y)
        1, 5, 6,
        1, 6, 2,

        // Bottom (-Y)
        0, 3, 7,
        0, 7, 4
    };

	return data;
}