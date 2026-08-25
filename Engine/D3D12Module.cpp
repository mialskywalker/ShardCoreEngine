#include "D3D12Module.h"
#include "Logger.h"

#include "ResourceModule.h"
#include "ReadData.h"

D3D12Module::D3D12Module() {}

D3D12Module::~D3D12Module() {}

bool D3D12Module::Init(HWND hwnd, uint32_t width, uint32_t height)
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
	ok = ok && CreateRootSignature();
	ok = ok && CreatePipelineStateObject();

	return ok;
}

void D3D12Module::BeginFrame()
{
	m_CommandAllocators[m_CurrentFrameIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentFrameIndex].Get(), m_PipelineStateObject.Get());
}

void D3D12Module::EndFrame()
{
	m_SwapChain->Present(1, 0);
	m_CurrentFrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Module::EnableDebugLayer()
{
	D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugInterface));
	m_DebugInterface->EnableDebugLayer();
	Logger::Log("Debug Layer enabled");
}

void D3D12Module::EnableInfoQueue()
{
	// Must have device created before enabling
	m_Device.As(&m_InfoQueue);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

bool D3D12Module::CreateDevice()
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

bool D3D12Module::CreateCommandQueue()
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

bool D3D12Module::CreateSwapChain()
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

bool D3D12Module::CreateDescriptorHeaps()
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

bool D3D12Module::CreateRenderTargetViews()
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

bool D3D12Module::CreateCommandAllocators()
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

bool D3D12Module::CreateCommandList()
{
	bool ok = SUCCEEDED(m_Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));

	if (ok)
		Logger::Log("Command List created");
	else
		Logger::Log("Command List creation failed");

	return ok;
}

bool D3D12Module::CreateFence()
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

bool D3D12Module::CreateRootSignature()
{
	bool ok = false;

	CD3DX12_ROOT_SIGNATURE_DESC desc = {};
	desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> blob;
	ok = SUCCEEDED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr));
	ok = ok && SUCCEEDED(m_Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

	if (ok)
		Logger::Log("Root Signature created");
	else
		Logger::Log("Root Signature creation failed");

	return ok;
}

bool D3D12Module::CreatePipelineStateObject()
{
	bool ok = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = m_RootSignature.Get();

	auto dataVS = DX::ReadData(L"VertexShader.cso");
	auto dataPS = DX::ReadData(L"PixelShader.cso");

	desc.VS = { dataVS.data(), dataVS.size() };
	desc.PS = { dataPS.data(), dataPS.size() };

	D3D12_INPUT_ELEMENT_DESC layout[] = { {"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

	desc.InputLayout = { layout, sizeof(layout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.NumRenderTargets = 1;
	desc.SampleDesc = { 1, 0 };
	desc.SampleMask = 0xffffffff;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	ok = SUCCEEDED(m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_PipelineStateObject)));

	if (ok)
		Logger::Log("Pipeline State Object created");
	else
		Logger::Log("Pipeline State Object creation failed");

	return ok;
}

void D3D12Module::WaitForGPU()
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

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::GetCurrentRTVHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += m_CurrentFrameIndex * m_RTVDescriptorSize;

	return handle;
}
