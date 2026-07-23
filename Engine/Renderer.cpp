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
	
	return ok;
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

	float clearColor[4] = {0.0f, 0.0f, 1.0f, 0.0f};
	m_D3D12Module->GetCommandList()->ClearRenderTargetView(m_D3D12Module->GetCurrentRTVHandle(), clearColor, 0, nullptr);

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