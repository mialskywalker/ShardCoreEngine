#include "ResourceModule.h"

ResourceModule::ResourceModule() {}

ResourceModule::~ResourceModule() {}

bool ResourceModule::Init(
	ID3D12Device5* device
	)
{
	if (!device)
		return false;

	m_Device = device;

	return true;
}

ComPtr<ID3D12Resource> ResourceModule::CreateUploadBuffer(size_t bufferSize, void* data)
{
	ComPtr<ID3D12Resource> uploadBuffer;

	// Buffer description
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	// Specify UPLOAD heap properties
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

	// Create the resource
	m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	// Get a CPU pointer to its memory
	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We won't read from it
	uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));

	// Copy application data into the GPU buffer
	memcpy(pData, data, bufferSize);

	// Unmap (invalidate the pointer)
	uploadBuffer->Unmap(0, nullptr);

	return uploadBuffer;

}

ComPtr<ID3D12Resource> ResourceModule::CreateDefaultBuffer(size_t bufferSize)
{
	// Vertex Buffer
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_RESOURCE_DESC vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES vertexProps(D3D12_HEAP_TYPE_DEFAULT);
	m_Device->CreateCommittedResource(&vertexProps, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

	return vertexBuffer;
}