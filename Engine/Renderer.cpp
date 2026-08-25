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
		Logger::Log("DirectX 12 Initialization success!");

	// ResourceModule init
	m_ResourceModule = std::make_unique<ResourceModule>();
	ok = ok && m_ResourceModule->Init(m_D3D12Module.get());

	// try to render a triangle
	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[3] =
	{
		{-1.0f, -1.0f, 0.0f}, // 0
		{ 0.0f,  1.0f, 0.0f}, // 1
		{ 1.0f, -1.0f, 0.0f}  // 2
	};

	m_vertexBuffer = m_ResourceModule->CreateDefaultBuffer(sizeof(vertices), vertices);
	vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = sizeof(vertices);

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



	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_D3D12Module->GetCurrentRTVHandle();

	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(m_D3D12Module->GetWidth()), float(m_D3D12Module->GetHeight()), 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, m_D3D12Module->GetWidth(), m_D3D12Module->GetHeight() };
	
	m_D3D12Module->GetCommandList()->RSSetViewports(1, &viewport);
	m_D3D12Module->GetCommandList()->RSSetScissorRects(1, &scissor);
	m_D3D12Module->GetCommandList()->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
	m_D3D12Module->GetCommandList()->ClearRenderTargetView(m_D3D12Module->GetCurrentRTVHandle(), clearColor, 0, nullptr);
	m_D3D12Module->GetCommandList()->SetGraphicsRootSignature(m_D3D12Module->GetRootSignature());
	m_D3D12Module->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_D3D12Module->GetCommandList()->IASetVertexBuffers(0, 1, &vbv);
	m_D3D12Module->GetCommandList()->DrawInstanced(3, 1, 0, 0);

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