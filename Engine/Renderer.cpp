#include "Renderer.h"
#include "Logger.h"

#include "ResourceModule.h"

Renderer::Renderer() {}

Renderer::~Renderer() 
{
	delete m_ResourceModule;
	m_ResourceModule = nullptr;
}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	// Window init
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;

	// DX12 init pipeline
	bool ok = true;

#if defined(_DEBUG)
	EnableDebugLayer();
#endif

	ok = ok && CreateDevice();
	if (ok)
		EnableInfoQueue();
	ok = ok && CreateCommandQueue();
	ok = ok && CreateSwapChain();
	ok = ok && CreateDescriptorHeaps();
	ok = ok && CreateRenderTargetViews();
	ok = ok && CreateCommandAllocators();
	ok = ok && CreateCommandList();
	ok = ok && CreateFence();

	if (ok)
		Logger::Log("DX12 initialization success");

	// ResourceModule init
	m_ResourceModule = new ResourceModule();
	if (m_ResourceModule)
		Logger::Log("ResourceModule initialzied");

	ok = ok && m_ResourceModule->Init(m_Device.Get());

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

	WaitForGPU();
	
	return ok;
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
		UploadToDefaultBuffer(
			bufferSize,
			(void*)testData);

	if (!defaultBuffer)
		return false;

	WaitForGPU();

	D3D12_RESOURCE_DESC readbackDesc =
		CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	CD3DX12_HEAP_PROPERTIES readbackHeapProperties(
		D3D12_HEAP_TYPE_READBACK);

	ComPtr<ID3D12Resource> readbackBuffer;

	HRESULT result = m_Device->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&readbackDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&readbackBuffer));

	if (FAILED(result))
		return false;

	result =
		m_CommandAllocators[m_CurrentFrameIndex]->Reset();

	if (FAILED(result))
		return false;

	result = m_CommandList->Reset(
		m_CommandAllocators[m_CurrentFrameIndex].Get(),
		nullptr);

	if (FAILED(result))
		return false;

	CD3DX12_RESOURCE_BARRIER toCopySource =
		CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_COPY_SOURCE);

	m_CommandList->ResourceBarrier(
		1,
		&toCopySource);

	m_CommandList->CopyBufferRegion(
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

	m_CommandList->ResourceBarrier(
		1,
		&toVertexBuffer);

	result = m_CommandList->Close();

	if (FAILED(result))
		return false;

	ID3D12CommandList* commandLists[] =
	{
		m_CommandList.Get()
	};

	m_CommandQueue->ExecuteCommandLists(
		1,
		commandLists);

	WaitForGPU();

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
	m_CommandAllocators[m_CurrentFrameIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentFrameIndex].Get(), nullptr);
}

void Renderer::Render()
{
	// PreRender
	PreRender();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &barrier);

	auto RTVHandle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	RTVHandle.ptr += m_CurrentFrameIndex * m_RTVDescriptorSize;

	m_CommandList->ClearRenderTargetView(RTVHandle, m_ClearColor, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	m_CommandList->Close();

	ID3D12CommandList* commandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, commandLists);

	// PostRender
	PostRender();

	// Flush
	WaitForGPU();
}

void Renderer::PostRender()
{
	m_SwapChain->Present(1, 0);
	m_CurrentFrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void Renderer::EnableDebugLayer()
{
	D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugInterface));
	m_DebugInterface->EnableDebugLayer();
	Logger::Log("Debug Layer enabled");
}

void Renderer::EnableInfoQueue()
{
	// Must have device created before enabling
	m_Device.As(&m_InfoQueue);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

bool Renderer::CreateDevice()
{
	// Create a factory
#if defined(_DEBUG)
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(&m_Factory));
#endif

	// Enumerate through adapters and choose the most suitable
	m_Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_Adapter));

	// Create a device
	bool ok = SUCCEEDED(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));

	if (ok)
		Logger::Log("Device created");
	else
		Logger::Log("Device creation failed");
	
	return ok;
}

bool Renderer::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	bool ok = SUCCEEDED(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

	if (ok)
		Logger::Log("Command Queue created");
	else
		Logger::Log("Command Queue creation failed");

	return ok;
}

bool Renderer::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = m_FramesInFlight;

	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain;
	bool ok = SUCCEEDED(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Hwnd, &desc, NULL, NULL, &swapChain));

	ok = ok && SUCCEEDED(swapChain.As(&m_SwapChain));

	if (ok)
		Logger::Log("Swap Chain created");
	else
		Logger::Log("Swap Chain creation failed");

	return ok;
}

bool Renderer::CreateDescriptorHeaps()
{
	// RTV
	D3D12_DESCRIPTOR_HEAP_DESC descRTV = {};
	descRTV.NumDescriptors = m_FramesInFlight;
	descRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	bool ok = SUCCEEDED(m_Device->CreateDescriptorHeap(&descRTV, IID_PPV_ARGS(&m_RTVDescriptorHeap)));
	
	if (ok)
		Logger::Log("RTV Descriptor Heap created");
	else
		Logger::Log("RTV Descriptor Heap creation failed");

	// to add depth/stencil etc.

	return ok;
}

bool Renderer::CreateRenderTargetViews()
{
	bool ok = true;

	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (unsigned i = 0; i < m_FramesInFlight; ++i)
	{
		ok = ok && SUCCEEDED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i])));
		if (!ok)
		{
			Logger::Log("RTVs Creation failed");
			return ok;
		}

		m_Device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, RTVHandle);
		RTVHandle.Offset(m_RTVDescriptorSize);
	}

	Logger::Log("RTVs created");
	return ok;
}

bool Renderer::CreateCommandAllocators()
{
	bool ok = true;

	for (int i = 0; i < m_FramesInFlight; ++i)
	{
		ok = ok && SUCCEEDED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i])));
		if (!ok)
		{
			Logger::Log("Command Allocators creation failed");
			return ok;
		}
	}

	Logger::Log("Command Allocators created");
	return ok;
}

bool Renderer::CreateCommandList()
{
	bool ok = SUCCEEDED(m_Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));

	if (ok)
		Logger::Log("Command List created");
	else
		Logger::Log("Command List creation failed");

	return ok;
}

bool Renderer::CreateFence()
{
	if (FAILED(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
	{
		return false;
	}

	// set 1 as starting sync point (value)
	m_FenceValue = 1;

	// windows event handle
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	Logger::Log("Fence + Event Created!");

	return m_FenceEvent != nullptr;
}

void Renderer::WaitForGPU()
{
	// tell the gpu queue when it reaches this value that the cpu can continue after that
	m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue);

	// check if the gpu has used this frame - if yes -> ensure it has finished
	if (m_Fence->GetCompletedValue() < m_FenceValue)
	{
		// signal when fence reaches a value
		m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
		// CPU blocked while GPU reaches that value
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	// start a new frame
	m_FenceValue++;
}

ComPtr<ID3D12Resource> Renderer::UploadToDefaultBuffer(size_t bufferSize, void* data)
{
	ComPtr<ID3D12Resource> stagingBuffer = m_ResourceModule->CreateUploadBuffer(bufferSize, data);
	ComPtr<ID3D12Resource> vertexBuffer = m_ResourceModule->CreateDefaultBuffer(bufferSize);

	// Prep Command List to record
	m_CommandAllocators[m_CurrentFrameIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentFrameIndex].Get(), nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	m_CommandList->ResourceBarrier(1, &barrier);

	// Copy Data
	m_CommandList->CopyResource(vertexBuffer.Get(), stagingBuffer.Get());

	// Transition
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_CommandList->ResourceBarrier(1, &barrier);
	m_CommandList->Close();

	// Execute Commands
	ID3D12CommandList* lists[] = { m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(1, lists);

	WaitForGPU();

	return vertexBuffer;
}