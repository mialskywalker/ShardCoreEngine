#pragma once
#include "windows.h"
#include <cstdint>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class ResourceModule
{
public:
	ResourceModule();
	~ResourceModule();

	bool Init(ID3D12Device5* device);

	ComPtr<ID3D12Resource> CreateUploadBuffer(size_t bufferSize, void* data);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(size_t bufferSize);

private:
	ID3D12Device5* m_Device;
};