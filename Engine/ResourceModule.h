#pragma once
#include "windows.h"
#include <cstdint>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class D3D12Module;

class ResourceModule
{
public:
	ResourceModule();
	~ResourceModule();

	bool Init(D3D12Module* d3d12);

	ComPtr<ID3D12Resource> CreateUploadBuffer(size_t bufferSize, void* data);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(size_t bufferSize, void* data);

private:
	D3D12Module* m_D3D12 = nullptr;
};