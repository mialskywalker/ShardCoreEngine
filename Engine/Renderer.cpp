#include "Renderer.h"
#include "Logger.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_Hwnd = hwnd;
	m_Width = width;
	m_Height = height;

	bool ok = CreateFactory();
	ok = ok && PickAdapter();
	ok = ok && CreateDevice();

	Logger::Log("Renderer initialization successful");

	return ok;
}

void Renderer::Render()
{
	
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