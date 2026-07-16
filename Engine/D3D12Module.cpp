#include "D3D12Module.h"

#include "Logger.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;

	bool ok = true;

#if defined(_DEBUG)
	ok = ok && EnableDebugLayer();
#endif

	ok = ok && CreateFactory();
	ok = ok && PickAdapter();
	ok = ok && CreateDevice();
	ok = ok && CreateCommandQueue();
	ok = ok && CreateSwapChain();
	ok = ok && CreateRTVDescriptorHeap();
	ok = ok && CreateRTV();
	ok = ok && CreateCommandAllocators();
	ok = ok && CreateCommandList();
	ok = ok && CreateFence();

	Logger::Log("Renderer initialization successful");

	return ok;
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

	// Render
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

bool Renderer::EnableDebugLayer()
{
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugController))))
	{
		m_DebugController->EnableDebugLayer();
		Logger::Log("Debug Layer Enabled!");
		return true;
	}

	return false;
}

bool Renderer::CreateFactory()
{
	UINT flags = 0;

#if defined(_DEBUG)
	flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	bool ok = SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_Factory)));

	if (ok)
	{
		Logger::Log("Created Factory");
	}
	else
	{
		Logger::Log("Factory creation FAILED");
	}

	return ok;
}

bool Renderer::PickAdapter()
{
	ComPtr<IDXGIAdapter1> adapter;

	for (UINT index = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(index, &adapter); ++index)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		adapter.As(&m_Adapter);
		Logger::Log("Compatible adapter found");
		return true;
	}

	Logger::Log("Compatible adapter not found");
	return false;
}

bool Renderer::CreateDevice()
{
	bool ok = SUCCEEDED(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));

	if (ok)
	{
		Logger::Log("Created Device");
	}
	else
	{
		Logger::Log("Device creation FAILED");
	}

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
	{
		Logger::Log("Created A Command Queue");
	}
	else
	{
		Logger::Log("Command Queue Creation FAILED");
	}

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
	desc.BufferCount = FrameCount;

	desc.Scaling = DXGI_SCALING_NONE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;

	bool ok = SUCCEEDED(m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Hwnd, &desc, nullptr, nullptr, &swapChain1));

	ok = ok && SUCCEEDED(swapChain1.As(&m_SwapChain));

	if (ok)
	{
		Logger::Log("Created Swap Chain");
	}
	else
	{
		Logger::Log("Swap Chain Creation FAILED");
	}

	return ok;
}

bool Renderer::CreateRTVDescriptorHeap()
{
	// Create a descriptor heap for RTV
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = FrameCount;

	bool ok = SUCCEEDED(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));

	if (ok)
	{
		Logger::Log("Descriptor Heap Created");
	}
	else
	{
		Logger::Log("Descriptor Heap Creation FAILED");
	}

	return ok;
}

bool Renderer::CreateRTV()
{
	// Take descriptor size (different depending on GPU/driver)
	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Take CPU handle at the beginning
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create RTV for each back buffer
	for (int i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]))))
		{
			Logger::Log("RTVs Creation FAILED");
			return false;
		}
		m_Device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(m_RTVDescriptorSize);
	}

	Logger::Log("RTVs Created");
	return true;
}

bool Renderer::CreateCommandAllocators()
{
	bool ok = true;

	// Create command allocators for each FrameCount to avoid resetting it while the GPU might use commands from it

	for (int i = 0; i < FrameCount; ++i)
	{
		ok = ok && SUCCEEDED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i])));
	}

	if (ok)
	{
		Logger::Log("Command Allocators Created");
	}
	else
	{
		Logger::Log("Command Allocators Creation FAILED");
	}

	return ok;
}

bool Renderer::CreateCommandList()
{
	bool ok = SUCCEEDED(m_Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));

	if (ok)
	{
		Logger::Log("Command List Created");
	}
	else
	{
		Logger::Log("Command List Creation FAILED");
	}

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