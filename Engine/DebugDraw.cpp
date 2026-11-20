#include "DebugDraw.h"
#include "D3D12Module.h"
#include "ReadData.h"
#include "Camera.h"
#include <algorithm>
#include <cstring>

bool DebugDraw::Init(D3D12Module& d3d12)
{
    m_D3D12 = &d3d12;

    if (!CreateVertexBuffer() ||
        !CreatePipelineState())
    {
        Shutdown();
        return false;
    }

    return true;
}

void DebugDraw::Shutdown()
{
    if (m_VertexBuffer && m_MappedData)
    {
        m_VertexBuffer->Unmap(0, nullptr);
        m_MappedData = nullptr;
    }

    m_VertexBuffer.Reset();
    m_VertexBufferView = {};

    m_LineVertices.clear();
    m_D3D12 = nullptr;
    m_PipelineState.Reset();
}

void DebugDraw::Render(Camera& camera)
{
    if (m_LineVertices.empty())
        return;

    auto* cmd = m_D3D12->GetCommandList();

    // Limit vertex count to buffer capacity
    size_t vertexCount = (std::min)(
        m_LineVertices.size(),
        static_cast<size_t>(MAX_VERTICES)
    );

    // Every line requires two vertices
    vertexCount -= vertexCount % 2;

    if (vertexCount == 0)
        return;

    UINT bufferSize = static_cast<UINT>(
        vertexCount * sizeof(Vertex)
        );

    // Update GPU upload buffer
    memcpy(
        m_MappedData,
        m_LineVertices.data(),
        bufferSize
    );

    // Set Debug Pipeline
    cmd->SetPipelineState(m_PipelineState.Get());

    cmd->SetGraphicsRootSignature(
        m_D3D12->GetRootSignature()
    );

    cmd->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_LINELIST
    );

    // View Projection Matrix
    Matrix viewProjection = (
        camera.GetView() *
        camera.GetProjection()
        ).Transpose();

    cmd->SetGraphicsRoot32BitConstants(
        0,
        sizeof(Matrix) / sizeof(UINT32),
        &viewProjection,
        0
    );

    // Bind Vertex Buffer
    auto vbv = m_VertexBufferView;
    vbv.SizeInBytes = bufferSize;

    cmd->IASetVertexBuffers(0, 1, &vbv);

    // Draw Lines
    cmd->DrawInstanced(
        static_cast<UINT>(vertexCount),
        1,
        0,
        0
    );

    // Prepare for next frame
    Clear();
}

void DebugDraw::DrawLine(const Vector3& start, const Vector3& end, const Vector3& color)
{
	Vertex first =
	{
		{ start.x, start.y, start.z },
		{ color.x, color.y, color.z }
	};

	Vertex second =
	{
		{ end.x, end.y, end.z },
		{ color.x, color.y, color.z }
	};

	m_LineVertices.push_back(first);
	m_LineVertices.push_back(second);
}

void DebugDraw::DrawGrid(int halfCells, float spacing, const Vector3& color)
{
    if (halfCells <= 0 || spacing <= 0.0f)
    {
        return;
    }

    float extent = halfCells * spacing;

    for (int i = -halfCells; i <= halfCells; ++i)
    {
        float offset = i * spacing;

        // Highlight main axes
        Vector3 xColor = (i == 0) ? Vector3(1.0f, 0.2f, 0.2f) : color;
        Vector3 zColor = (i == 0) ? Vector3(0.2f, 0.4f, 1.0f) : color;

        // Lines parallel to X
        DrawLine(Vector3(-extent, 0.0f, offset), Vector3(extent, 0.0f, offset), xColor);

        // Lines parallel to Z
        DrawLine(Vector3(offset, 0.0f, -extent), Vector3(offset, 0.0f, extent), zColor);
    }
}

void DebugDraw::Clear()
{
	m_LineVertices.clear();
}

bool DebugDraw::CreateVertexBuffer()
{
    auto* device = m_D3D12->GetDevice();

    const UINT bufferSize = sizeof(Vertex) * MAX_VERTICES;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_VertexBuffer)
    );

    if (FAILED(hr))
        return false;

    // Persistent Mapping
    D3D12_RANGE readRange{ 0, 0 };

    hr = m_VertexBuffer->Map(
        0,
        &readRange,
        reinterpret_cast<void**>(&m_MappedData)
    );

    if (FAILED(hr))
        return false;

    // Vertex Buffer View
    m_VertexBufferView.BufferLocation =
        m_VertexBuffer->GetGPUVirtualAddress();

    m_VertexBufferView.StrideInBytes =
        sizeof(Vertex);

    m_VertexBufferView.SizeInBytes =
        bufferSize;

    return true;
}

bool DebugDraw::CreatePipelineState()
{
    auto* device = m_D3D12->GetDevice();

    auto dataVS = DX::ReadData(L"VertexShader.cso");
    auto dataPS = DX::ReadData(L"PixelShader.cso");

    D3D12_INPUT_ELEMENT_DESC layout[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

    desc.pRootSignature = m_D3D12->GetRootSignature();

    desc.VS = { dataVS.data(), dataVS.size() };
    desc.PS = { dataPS.data(), dataPS.size() };

    desc.InputLayout =
    {
        layout,
        _countof(layout)
    };

    desc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    desc.NumRenderTargets = 1;
    desc.SampleDesc = { 1, 0 };
    desc.SampleMask = UINT_MAX;

    desc.RasterizerState =
        CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    desc.RasterizerState.CullMode =
        D3D12_CULL_MODE_NONE;

    desc.DepthStencilState =
        CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    desc.BlendState =
        CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    return SUCCEEDED(
        device->CreateGraphicsPipelineState(
            &desc,
            IID_PPV_ARGS(&m_PipelineState)
        )
    );
}