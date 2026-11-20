#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class D3D12Module;
class ShaderDescriptorModule;

class RenderTexture
{
public:
    ~RenderTexture();

    bool Init(D3D12Module& d3d12, ShaderDescriptorModule& descriptors, uint32_t width, uint32_t height);
    bool Resize(uint32_t width, uint32_t height);

    ID3D12Resource* GetResource() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

private:
    bool CreateDepthBuffer(uint32_t width, uint32_t height);

private:
    D3D12Module* m_D3D12 = nullptr;
    ShaderDescriptorModule* m_Descriptors = nullptr;

    int m_DescriptorIndex = -1;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle{};

    D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_SRVHandle{};

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
};