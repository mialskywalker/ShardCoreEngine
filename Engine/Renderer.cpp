#include "Renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer() 
{
	if (m_CommandQueue && m_Fence)
		WaitForGPU();

	if (m_FenceEvent)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}
}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;
	
	bool ok = CreateFactory();
	ok = ok && PickAdapter();
	ok = ok && CreateDevice();
	ok = ok && CreateCommandQueue();
	ok = ok && CreateSwapChain();
	
	m_CurrentFrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	ok = ok && CreateRenderTargets();
	ok = ok && CreateCommandAllocators();
	ok = ok && CreateCommandList();
	ok = ok && CreateFence();

	WaitForGPU();

	return ok;
}

void Renderer::Render()
{
	m_CommandAllocators[m_CurrentFrameIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentFrameIndex].Get(), nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &barrier);

	auto RTVhandle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	RTVhandle.ptr += m_CurrentFrameIndex * m_RTVDescriptorSize;

	float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };
	m_CommandList->ClearRenderTargetView(RTVhandle, clearColor, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	m_CommandList->Close();

	ID3D12CommandList* commandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, commandLists);

	m_SwapChain->Present(1, 0);
	m_CurrentFrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	WaitForGPU();
}

// Create a factory (physical GPU)
bool Renderer::CreateFactory()
{
	UINT flags = 0;

#if defined(_DEBUG)
	flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	bool ok = SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_Factory)));
	return ok;
}

// Pick adapter
bool Renderer::PickAdapter()
{
	ComPtr<IDXGIAdapter1> adapter;

	for (UINT index = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(index, &adapter); ++index)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		m_Adapter = adapter;
		return true;
	}
	
	return false;
}

// Create a device
bool Renderer::CreateDevice()
{
	return SUCCEEDED(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
}

// Create a command queue
bool Renderer::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	bool ok = SUCCEEDED(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
	return ok;
}

// Create a swap chain
bool Renderer::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FrameCount;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;
	
	ComPtr<IDXGISwapChain1> swapChain1;
	bool ok = SUCCEEDED(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Hwnd, &desc, nullptr, nullptr, &swapChain1));

	ok = ok && SUCCEEDED(swapChain1.As(&m_SwapChain));

	// Stops default alt + enter fullscreen behaviour
	m_Factory->MakeWindowAssociation(m_Hwnd, DXGI_MWA_NO_ALT_ENTER);
	
	return ok;
}

// Create render targets
bool Renderer::CreateRenderTargets()
{
	// Create a descriptor heap for RTV
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = FrameCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	bool ok = SUCCEEDED(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));

	// Take descriptor size (different depending on GPU/driver)
	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Take CPU handle at the beginning
	auto handle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Create RTV for each back buffer
	for (uint32_t i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]))))
			return false;

		m_Device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, handle);
		handle.ptr += m_RTVDescriptorSize;
	}

	return ok;
}

// Create command allocators
bool Renderer::CreateCommandAllocators()
{
	// Create command allocators for each FrameCount to avoid resetting it while the GPU might use commands from it
	for (uint32_t i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i]))))
			return false;
	}

	return true;
}

// Create a command list
bool Renderer::CreateCommandList()
{

	if (FAILED(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_CommandList))))
		return false;
	
	m_CommandList->Close();

	return true;
}

// Create a fence
bool Renderer::CreateFence()
{
	if (FAILED(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
		return false;

	// set 1 as starting sync point (value)
	m_FenceValue = 1;

	// windows event handle
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	
	return m_FenceEvent != nullptr;
}

// GPU Synchronization
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