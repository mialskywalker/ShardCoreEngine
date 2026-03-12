#include "Common.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ResourceModule.h"
#include <vector>

#include "DirectXTex.h"

ResourceModule::ResourceModule() {}

ResourceModule::~ResourceModule() {}

// Create a command allocator and command list
bool ResourceModule::init()
{
	D3D12Module* d3d12 = app->getD3D12();
	bool ok = SUCCEEDED(d3d12->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ok = ok && SUCCEEDED(d3d12->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	return ok;
}

// CPU-writeable buffer
ComPtr<ID3D12Resource> ResourceModule::createUploadBuffer(size_t bufferSize, void* data)
{
	D3D12Module* d3d12 = app->getD3D12();

	// create an empty smart pointer
	ComPtr<ID3D12Resource> buffer;

	// set it as a buffer
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	// set heap as upload type
	CD3DX12_HEAP_PROPERTIES props(D3D12_HEAP_TYPE_UPLOAD);

	// create a heap and resource linked to it
	d3d12->getDevice()->CreateCommittedResource(
		&props,
		D3D12_HEAP_FLAG_NONE,
		&desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	// get a pointer ready for mapping
	BYTE* pData = nullptr;

	// (0, 0) means it wont read from this resource, only write to it
	CD3DX12_RANGE readRange(0, 0);

	// get cpu pointer to memory available to cpu
	buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));

	// copy data to upload buffer
	memcpy(pData, data, bufferSize);

	// end of cpu access to the memory
	buffer->Unmap(0, nullptr);

	return buffer;
}