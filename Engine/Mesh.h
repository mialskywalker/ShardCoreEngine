#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cstdint>

#include "Vertex.h"

class ResourceModule;

class Mesh
{
public:
    bool Init(ResourceModule& resourceModule, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices = {});

    const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const;
    uint32_t GetVertexCount() const;

    const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;
    uint32_t GetIndexCount() const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};
    uint32_t m_VertexCount = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};
    uint32_t m_IndexCount = 0;
};