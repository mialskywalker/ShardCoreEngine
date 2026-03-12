#include "Common.h"
#include "D3D12Module.h"

// Save the window handle used for swap chain creation and window size queries
D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module() 
{
	// ensure GPU has finished all submitted work
	flush();

	// Release the fence event handle
	if (fenceEvent)
	{
		CloseHandle(fenceEvent);
		fenceEvent = NULL;
	}
}

// Initialize the complete D3D12 rendering context
// device, command queue, swap chain, render targets, command infrastructure
// syncronization objects, and depth-stencil resources
bool D3D12Module::init()
{
	bool ok = false;

	getWindowSize(windowWidth, windowHeight);

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	ok = createFactory();
	ok = ok && createDevice();
#if defined(_DEBUG)
	setupInfoQueue();
#endif
	ok = ok && createCommandQueue();
	ok = ok && createSwapChain();
	ok = ok && createRTVDescriptorHeap();
	createRTV();
	ok = ok && createCommandAllocators();
	ok = ok && createCommandList();
	ok = ok && createFence();
	ok = ok && createDepthStencilBuffer();
	ok = ok && createDSVDescriptorHeap();
	createDSV();

	return ok;
}

// Prepare the current frame
// wait until the current back buffer is no longer used by the GPU
// reset its command allocator and reset the main command list for recording
void D3D12Module::preRender()
{
	// take the current back buffer index
	currentBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// check if the gpu has used this frame - if yes -> ensure it has finished
	if (fenceValues[currentBufferIndex] != 0)
	{
		// signal when fence reaches a value
		fence->SetEventOnCompletion(fenceValues[currentBufferIndex], fenceEvent);
		// CPU blocked while GPU reaches that value
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	// must be reset after gpu is finished with it
	commandAllocators[currentBufferIndex]->Reset();
	// prepare the command list for taking new commands
	commandList->Reset(commandAllocators[currentBufferIndex].Get(), nullptr);
}

// Finish recording the frame commands, submit them to the GPU
// present the current back buffer and signal a fence value for synchronization
void D3D12Module::postRender()
{
	// close and execute the command list
	commandList->Close();
	ID3D12CommandList* lists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, lists);

	// tell the swap chain to show the current back buffer
	swapChain->Present(vsync ? 1 : 0, 0);
	// this frame/buffer is linked to this fence value
	fenceValues[currentBufferIndex] = ++fenceValue;
	// tell the gpu queue when it reaches this value that the cpu can continue after that
	commandQueue->Signal(fence.Get(), fenceValue);
}

// Force the CPU to wait until the GPU finishes all previously submitted work
void D3D12Module::flush()
{
	if (!commandQueue || !fence || !fenceEvent)
		return;

	commandQueue->Signal(fence.Get(), ++fenceValue);
	fence->SetEventOnCompletion(fenceValue, fenceEvent);
	WaitForSingleObject(fenceEvent, INFINITE);
}

// Enable debug layer for validation errors and warnings in debug mode
void D3D12Module::enableDebugLayer()
{
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
}

// Create a factory (physical gpu)
bool D3D12Module::createFactory()
{
	bool ok = false;
#if defined(_DEBUG)
	ok = SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));
#else
	ok = SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
#endif

	return ok;
}

// Create a device (iterating through available gpus and choosing most suitable)
bool D3D12Module::createDevice()
{
	bool ok = false;
	ok = SUCCEEDED(factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
	ok = ok && SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

	return ok;
}

// Configure the debug info queue to break execution on severe validation messages.
void D3D12Module::setupInfoQueue()
{
	if (SUCCEEDED(device.As(&infoQueue)))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	}
}

// Create a command queue to execute command lists
bool D3D12Module::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	bool ok = SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

	return ok;
}

// Create a swap chain to manage backbuffers
bool D3D12Module::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAMES_IN_FLIGHT;

	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	bool ok = SUCCEEDED(factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1));

	ok = ok && SUCCEEDED(swapChain1.As(&swapChain));

	return ok;
}

// Create a RenderTargetView DescriptorHeap
bool D3D12Module::createRTVDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FRAMES_IN_FLIGHT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	bool ok = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&RTVdescriptorHeap)));

	return ok;
}

// Create RenderTargetViews
void D3D12Module::createRTV()
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle(RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	unsigned descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
		handle.ptr += descriptorSize;
	}
}

// Create one command allocator per frame-in-flight so each buffered frame
// can manage its own command memory safely
bool D3D12Module::createCommandAllocators()
{
	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]))))
		{
			return false;
		}
	}

	return true;
}

// Create the main graphics command list and close it immediately
// It will be reset and recorded again every frame
bool D3D12Module::createCommandList()
{
	bool ok = false;

	ok = SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)));
	commandList->Close();

	return ok;
}

// Create the fence object and an OS event used to wait for GPU completion
bool D3D12Module::createFence()
{
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
		return false;

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return fenceEvent != NULL;
}

// Create a DepthStencil Buffer
bool D3D12Module::createDepthStencilBuffer()
{
	CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clear = {};
	clear.Format = DXGI_FORMAT_D32_FLOAT;
	clear.DepthStencil.Depth = 1.0f;
	clear.DepthStencil.Stencil = 0;

	bool ok = SUCCEEDED(device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear, IID_PPV_ARGS(&depthStencilBuffer)));

	return ok;
}

// Create DSV Descriptor Heap
bool D3D12Module::createDSVDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	bool ok = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DSVdescriptorHeap)));

	return ok;
}

// Create a DSV
void D3D12Module::createDSV()
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle(DSVdescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, handle);
}

// Get Window Size
void D3D12Module::getWindowSize(unsigned& width, unsigned& height)
{
	RECT rect = {};
	GetClientRect(hWnd, &rect);

	width = unsigned(rect.right - rect.left);
	height = unsigned(rect.bottom - rect.top);
}

// Recreate all window-size-dependent resources when the client area changes
// back buffers, RTVs, depth buffer, and DSV.
void D3D12Module::resize()
{
	unsigned width, height;
	getWindowSize(width, height);

	if (width == 0 || height == 0)
		return;

	if (windowWidth != width || windowHeight != height)
	{
		windowWidth = width;
		windowHeight = height;
		flush();

		for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
		{
			backBuffers[i].Reset();
			depthStencilBuffer.Reset();
			fenceValues[i] = 0;
		}

		DXGI_SWAP_CHAIN_DESC desc = {};
		swapChain->GetDesc(&desc);

		swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, windowWidth, windowHeight, desc.BufferDesc.Format, desc.Flags);
		currentBufferIndex = swapChain->GetCurrentBackBufferIndex();

		createRTV();
		createDepthStencilBuffer();
		createDSV();
	}
}

// Get the RTV handle for the current back buffer.
D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::getRTVCPUDescriptorHandle()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		currentBufferIndex,
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	);
}

// Get the DSV handle for the depth buffer.
D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::getDSVCPUDescriptorHandle()
{
	return DSVdescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}