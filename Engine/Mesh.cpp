#include "Mesh.h"
#include "ResourceModule.h"

bool Mesh::Init(ResourceModule& resourceModule, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	if (vertices.empty())
	{
		return false;
	}

	// Calculate buffer size
	uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));

	// Create GPU vertex buffer
	m_VertexBuffer = resourceModule.CreateDefaultBuffer(vertexBufferSize, vertices.data());
	if (!m_VertexBuffer)
	{
		return false;
	}

	// Setup VBV
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	m_VertexBufferView.SizeInBytes = vertexBufferSize;

	// Store vertex count
	m_VertexCount = static_cast<uint32_t>(vertices.size());

	if (!indices.empty())
	{
		// Create Index Buffer

		// Calculate buffer size
		uint32_t indexBufferSize = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));

		// Create GPU index buffer
		m_IndexBuffer = resourceModule.CreateDefaultBuffer(indexBufferSize, indices.data(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
		if (!m_IndexBuffer)
		{
			return false;
		}

		// Setup IBV
		m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.SizeInBytes = indexBufferSize;
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		m_IndexCount = static_cast<uint32_t>(indices.size());
	}

	return true;
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBufferView() const
{
	return m_VertexBufferView;
}

uint32_t Mesh::GetVertexCount() const
{
	return m_VertexCount;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView() const
{
	return m_IndexBufferView;
}

uint32_t Mesh::GetIndexCount() const
{
	return m_IndexCount;
}