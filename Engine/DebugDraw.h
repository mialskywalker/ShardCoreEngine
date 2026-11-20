#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include "d3dx12.h"
#include <SimpleMath.h>
#include <vector>

#include "Vertex.h"

class D3D12Module;
class Camera;

using DirectX::SimpleMath::Vector3;

class DebugDraw
{
public:
    bool Init(D3D12Module& d3d12);
    void Shutdown();

    void Render(Camera& camera);

    void DrawLine(const Vector3& start, const Vector3& end, const Vector3& color);
    void DrawGrid(int halfCells, float spacing, const Vector3& color);

    void Clear();

public:
    const std::vector<Vertex>& GetVertices() const { return m_LineVertices; }

private:
    bool CreateVertexBuffer();
    bool CreatePipelineState();

private:
    std::vector<Vertex> m_LineVertices;
    D3D12Module* m_D3D12 = nullptr;

    static constexpr uint32_t MAX_VERTICES = 8192;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};

    uint8_t* m_MappedData = nullptr;
};