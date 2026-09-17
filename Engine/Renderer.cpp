#include "Renderer.h"
#include "Logger.h"

#include "D3D12Module.h"
#include "ResourceModule.h"
#include <SimpleMath.h>

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
	//struct Vertex
	//{
	//	float position[3];
	//};

	//Vertex vertices[3] =
	//{
	//	{-1.0f, -1.0f, 0.0f}, // 0
	//	{ 0.0f,  1.0f, 0.0f}, // 1
	//	{ 1.0f, -1.0f, 0.0f}  // 2
	//};

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	Vertex vertices[3] =
	{ 
		{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}} 
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

	// MVP TEST
	DirectX::SimpleMath::Matrix model = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix model2 = DirectX::SimpleMath::Matrix::CreateTranslation(1.0f, -1.0f, -1.0f);
	DirectX::SimpleMath::Matrix view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(0.0f, 10.0f, 10.0f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::Up);

	float aspect = float(m_D3D12Module->GetWidth()) / float(m_D3D12Module->GetHeight());
	float fov = DirectX::XM_PIDIV4;

	DirectX::SimpleMath::Matrix projection = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(fov, aspect, 0.1f, 1000.0f);

	DirectX::SimpleMath::Matrix mvp = (model * view * projection).Transpose();
	DirectX::SimpleMath::Matrix mvp2 = (model2 * view * projection).Transpose();
	// MVP TEST END

	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_D3D12Module->GetCurrentRTVHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_D3D12Module->GetDSVHandle();

	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(m_D3D12Module->GetWidth()), float(m_D3D12Module->GetHeight()), 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, m_D3D12Module->GetWidth(), m_D3D12Module->GetHeight() };
	
	m_D3D12Module->GetCommandList()->RSSetViewports(1, &viewport);
	m_D3D12Module->GetCommandList()->RSSetScissorRects(1, &scissor);
	m_D3D12Module->GetCommandList()->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	m_D3D12Module->GetCommandList()->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	m_D3D12Module->GetCommandList()->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	m_D3D12Module->GetCommandList()->SetGraphicsRootSignature(m_D3D12Module->GetRootSignature());
	m_D3D12Module->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_D3D12Module->GetCommandList()->IASetVertexBuffers(0, 1, &vbv);

	// triangle 1
	m_D3D12Module->GetCommandList()->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(UINT32), &mvp, 0); // temp
	m_D3D12Module->GetCommandList()->DrawInstanced(3, 1, 0, 0);

	// triangle 2
	m_D3D12Module->GetCommandList()->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(UINT32), &mvp2, 0); // temp
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