#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class D3D12Module;

class ShaderDescriptorModule
{
public:
	ShaderDescriptorModule();
	~ShaderDescriptorModule();

	bool Init(D3D12Module& d3d12);
	void Shutdown();

	int AllocateDescriptor();
	void FreeDescriptor(UINT index);

	ID3D12DescriptorHeap* GetHeap() const { return m_SRVDescriptorHeap.Get(); }
	UINT GetDescriptorSize() const { return m_DescriptorSize; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;

private:
	ComPtr<ID3D12DescriptorHeap>				m_SRVDescriptorHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE					m_GpuStart = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE					m_CpuStart = { 0 };
	UINT										m_DescriptorSize = 0;

	int											m_HeapCapacity = 256;
	int											m_NextFreeIndex = 0;

	std::vector<UINT>							m_FreeIndices;
};