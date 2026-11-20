#include "RenderTexture.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "d3dx12.h"

RenderTexture::~RenderTexture()
{
    if (m_Descriptors && m_DescriptorIndex >= 0)
    {
        m_Descriptors->FreeDescriptor(static_cast<UINT>(m_DescriptorIndex));
    }
}

bool RenderTexture::Init(D3D12Module& d3d12, ShaderDescriptorModule& descriptors, uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0 || m_Texture)
    {
        return false;
    }

    m_D3D12 = &d3d12;
    m_Descriptors = &descriptors;

    ID3D12Device* device = d3d12.GetDevice();

    // Create RTV Descriptor Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap))))
    {
        return false;
    }

    m_RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

    // Render Target Texture
    D3D12_RESOURCE_DESC textureDesc =
        CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            width,
            height,
            1,
            1,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearValue.Color[0] = 0.1f;
    clearValue.Color[1] = 0.1f;
    clearValue.Color[2] = 0.1f;
    clearValue.Color[3] = 1.0f;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&m_Texture))))
    {
        return false;
    }

    // Create RTV
    device->CreateRenderTargetView(m_Texture.Get(), nullptr, m_RTVHandle);

    // Allocate SRV Descriptor
    m_DescriptorIndex = descriptors.AllocateDescriptor();

    if (m_DescriptorIndex < 0)
    {
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, descriptors.GetCPUHandle(m_DescriptorIndex));

    m_SRVHandle = descriptors.GetGPUHandle(m_DescriptorIndex);
    m_Width = width;
    m_Height = height;

    if (!CreateDepthBuffer(width, height))
    {
        return false;
    }

    return true;
}

bool RenderTexture::CreateDepthBuffer(uint32_t width, uint32_t height)
{
    auto* device = m_D3D12->GetDevice();

    // Create DSV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DSVHeap))))
    {
        return false;
    }

    m_DSVHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Depth texture
    auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_D32_FLOAT,
        width,
        height,
        1,
        1,
        1,
        0,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
    );

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    if (FAILED(device->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_DepthBuffer))))
    {
        return false;
    }

    device->CreateDepthStencilView(m_DepthBuffer.Get(), nullptr, m_DSVHandle);

    return true;
}

ID3D12Resource* RenderTexture::GetResource() const
{
    return m_Texture.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetRTV() const
{
    return m_RTVHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTexture::GetSRV() const
{
    return m_SRVHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetDSV() const
{
    return m_DSVHandle;
}