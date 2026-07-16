#include "Renderer.h"
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
	EnableDebugLayer();
#endif

	ok = ok && CreateDevice();
	if (ok)
		EnableInfoQueue();

	return ok;
}

void Renderer::Render()
{

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
		Logger::Log("Device could not be created");
	
	return ok;
}