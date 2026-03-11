#include "Common.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module() 
{
	// flush();
}

bool D3D12Module::init()
{
	bool ok = false;

	// get window size first

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	ok = createFactory();
	ok = ok && createDevice();
#if defined(_DEBUG)
	setupInfoQueue();
#endif

	return ok;
}

// Enable debug layer for debugging information
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
	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	ok = SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

	return ok;
}

// Setup info queue for breaking when error occurs
void D3D12Module::setupInfoQueue()
{
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}