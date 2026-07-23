#include "ResourceModule.h"
#include "Renderer.h"
#include "D3D12Module.h"

ResourceModule::ResourceModule() {}

ResourceModule::~ResourceModule() {}

bool ResourceModule::Init(D3D12Module* d3d12)
{
	if (!d3d12)
		return false;

	m_D3D12 = d3d12;
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
	m_D3D12->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

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

ComPtr<ID3D12Resource> ResourceModule::CreateDefaultBuffer(size_t bufferSize, void* data)
{
	// Staging Buffer
	ComPtr<ID3D12Resource> stagingBuffer;
	D3D12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES stagingProps(D3D12_HEAP_TYPE_UPLOAD);
	m_D3D12->GetDevice()->CreateCommittedResource(&stagingProps, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));

	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	stagingBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	memcpy(pData, data, bufferSize);
	stagingBuffer->Unmap(0, nullptr);

	// Vertex Buffer
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_RESOURCE_DESC vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES vertexProps(D3D12_HEAP_TYPE_DEFAULT);
	m_D3D12->GetDevice()->CreateCommittedResource(&vertexProps, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));
	
	// Prep Command List to record
	m_D3D12->GetCurrentCommandAllocator()->Reset();
	m_D3D12->GetCommandList()->Reset(m_D3D12->GetCurrentCommandAllocator(), nullptr);
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	m_D3D12->GetCommandList()->ResourceBarrier(1, &barrier);
	
	// Copy Data
	m_D3D12->GetCommandList()->CopyResource(vertexBuffer.Get(), stagingBuffer.Get());
	
	// Transition
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_D3D12->GetCommandList()->ResourceBarrier(1, &barrier);
	m_D3D12->GetCommandList()->Close();
	
	// Execute Commands
	ID3D12CommandList* lists[] = { m_D3D12->GetCommandList() };
	m_D3D12->GetCommandQueue()->ExecuteCommandLists(1, lists);
	
	m_D3D12->WaitForGPU();

	return vertexBuffer;
}