#include "ShaderDescriptorModule.h"
#include "D3D12Module.h"
#include <algorithm>

ShaderDescriptorModule::ShaderDescriptorModule() {}

ShaderDescriptorModule::~ShaderDescriptorModule() {}

bool ShaderDescriptorModule::Init(D3D12Module& d3d12)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = m_HeapCapacity;

	bool ok = SUCCEEDED(d3d12.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_SRVDescriptorHeap)));
	if (!ok)
	{
		return false;
	}

	m_DescriptorSize = d3d12.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_GpuStart = m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_CpuStart = m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_NextFreeIndex = 0;
	m_FreeIndices.clear();

	return ok;
}

void ShaderDescriptorModule::Shutdown()
{
	m_SRVDescriptorHeap.Reset();
	m_GpuStart = { 0 };
	m_CpuStart = { 0 };
	m_DescriptorSize = 0;
	m_NextFreeIndex = 0;
	m_FreeIndices.clear();
}

int ShaderDescriptorModule::AllocateDescriptor()
{
	if (!m_FreeIndices.empty())
	{
		int index = m_FreeIndices.back();
		m_FreeIndices.pop_back();
		return index;
	}

	if (m_NextFreeIndex < m_HeapCapacity)
	{
		return m_NextFreeIndex++;
	}

	return -1;
}

void ShaderDescriptorModule::FreeDescriptor(UINT index)
{
	auto it = std::find(m_FreeIndices.begin(), m_FreeIndices.end(), index);

	if (it == m_FreeIndices.end() && index < m_NextFreeIndex)
	{
		m_FreeIndices.push_back(index);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderDescriptorModule::GetCPUHandle(UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_CpuStart;

	handle.ptr += index * m_DescriptorSize;

	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderDescriptorModule::GetGPUHandle(UINT index) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_GpuStart;

	handle.ptr += index * m_DescriptorSize;

	return handle;
}
