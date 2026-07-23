#include "Renderer.h"
#include "Logger.h"

#include "D3D12Module.h"
#include "ResourceModule.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	// D3D12Module init
	m_D3D12Module = std::make_unique<D3D12Module>();
	bool ok = m_D3D12Module->Init(hwnd, width, height);
	if (ok)
		Logger::Log("DX12 initialization success");

	// ResourceModule init
	m_ResourceModule = std::make_unique<ResourceModule>();
	ok = ok && m_ResourceModule->Init(m_D3D12Module.get());

	const bool uploadPassed =
		TestUploadBuffer();

	const bool defaultPassed =
		TestDefaultBuffer();

	if (!uploadPassed)
	{
		// Red
		m_ClearColor[0] = 1.0f;
		m_ClearColor[1] = 0.0f;
		m_ClearColor[2] = 0.0f;
		m_ClearColor[3] = 1.0f;
	}
	else if (!defaultPassed)
	{
		// Yellow
		m_ClearColor[0] = 1.0f;
		m_ClearColor[1] = 1.0f;
		m_ClearColor[2] = 0.0f;
		m_ClearColor[3] = 1.0f;
	}
	else
	{
		// Green
		m_ClearColor[0] = 0.0f;
		m_ClearColor[1] = 1.0f;
		m_ClearColor[2] = 0.0f;
		m_ClearColor[3] = 1.0f;
	}

	m_D3D12Module->WaitForGPU();
	
	return true;
}

bool Renderer::TestUploadBuffer()
{
	const uint32_t testData[] =
	{
		10,
		25,
		50,
		100,
		0x12345678
	};

	constexpr size_t bufferSize = sizeof(testData);
	constexpr size_t elementCount =
		sizeof(testData) / sizeof(testData[0]);

	ComPtr<ID3D12Resource> uploadBuffer =
		m_ResourceModule->CreateUploadBuffer(
			bufferSize,
			(void*)testData);

	if (!uploadBuffer)
		return false;

	void* mappedMemory = nullptr;

	CD3DX12_RANGE readRange(
		0,
		bufferSize);

	HRESULT result = uploadBuffer->Map(
		0,
		&readRange,
		&mappedMemory);

	if (FAILED(result) || !mappedMemory)
		return false;

	const uint32_t* uploadedData =
		static_cast<const uint32_t*>(mappedMemory);

	bool testPassed = true;

	for (size_t i = 0; i < elementCount; ++i)
	{
		if (uploadedData[i] != testData[i])
		{
			testPassed = false;
			break;
		}
	}

	CD3DX12_RANGE writtenRange(0, 0);

	uploadBuffer->Unmap(
		0,
		&writtenRange);

	return testPassed;
}

bool Renderer::TestDefaultBuffer()
{
	const uint32_t testData[] =
	{
		10,
		25,
		50,
		100,
		0x12345678
	};

	constexpr size_t bufferSize = sizeof(testData);
	constexpr size_t elementCount =
		sizeof(testData) / sizeof(testData[0]);

	ComPtr<ID3D12Resource> defaultBuffer =
		m_ResourceModule->CreateDefaultBuffer(
			bufferSize,
			(void*)testData);

	if (!defaultBuffer)
		return false;

	m_D3D12Module->WaitForGPU();

	D3D12_RESOURCE_DESC readbackDesc =
		CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	CD3DX12_HEAP_PROPERTIES readbackHeapProperties(
		D3D12_HEAP_TYPE_READBACK);

	ComPtr<ID3D12Resource> readbackBuffer;

	HRESULT result = m_D3D12Module->GetDevice()->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&readbackDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&readbackBuffer));

	if (FAILED(result))
		return false;

	result =
		m_D3D12Module->GetCurrentCommandAllocator()->Reset();

	if (FAILED(result))
		return false;

	result = m_D3D12Module->GetCommandList()->Reset(
		m_D3D12Module->GetCurrentCommandAllocator(),
		nullptr);

	if (FAILED(result))
		return false;

	CD3DX12_RESOURCE_BARRIER toCopySource =
		CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_COPY_SOURCE);

	m_D3D12Module->GetCommandList()->ResourceBarrier(
		1,
		&toCopySource);

	m_D3D12Module->GetCommandList()->CopyBufferRegion(
		readbackBuffer.Get(),
		0,
		defaultBuffer.Get(),
		0,
		bufferSize);

	CD3DX12_RESOURCE_BARRIER toVertexBuffer =
		CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_D3D12Module->GetCommandList()->ResourceBarrier(
		1,
		&toVertexBuffer);

	result = m_D3D12Module->GetCommandList()->Close();

	if (FAILED(result))
		return false;

	ID3D12CommandList* commandLists[] =
	{
		m_D3D12Module->GetCommandList()
	};

	m_D3D12Module->GetCommandQueue()->ExecuteCommandLists(
		1,
		commandLists);

	m_D3D12Module->WaitForGPU();

	void* mappedMemory = nullptr;

	CD3DX12_RANGE readRange(
		0,
		bufferSize);

	result = readbackBuffer->Map(
		0,
		&readRange,
		&mappedMemory);

	if (FAILED(result) || !mappedMemory)
		return false;

	const uint32_t* copiedData =
		static_cast<const uint32_t*>(mappedMemory);

	bool testPassed = true;

	for (size_t i = 0; i < elementCount; ++i)
	{
		if (copiedData[i] != testData[i])
		{
			testPassed = false;
			break;
		}
	}

	CD3DX12_RANGE writtenRange(0, 0);

	readbackBuffer->Unmap(
		0,
		&writtenRange);

	return testPassed;
}

void Renderer::PreRender()
{
	m_D3D12Module->BeginFrame();
}

void Renderer::Render()
{
	// PreRender
	PreRender();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_D3D12Module->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_D3D12Module->GetCommandList()->ResourceBarrier(1, &barrier);

	m_D3D12Module->GetCommandList()->ClearRenderTargetView(m_D3D12Module->GetCurrentRTVHandle(), m_ClearColor, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_D3D12Module->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_D3D12Module->GetCommandList()->ResourceBarrier(1, &barrier);

	m_D3D12Module->GetCommandList()->Close();

	ID3D12CommandList* commandLists[] = { m_D3D12Module->GetCommandList() };
	m_D3D12Module->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// PostRender
	PostRender();

	// Flush
	m_D3D12Module->WaitForGPU();
}

void Renderer::PostRender()
{
	m_D3D12Module->EndFrame();
}