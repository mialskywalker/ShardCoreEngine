#include "Renderer.h"
#include "Logger.h"

#include "D3D12Module.h"
#include "ResourceModule.h"
#include "ShaderDescriptorModule.h"
#include "Camera.h"
#include "Scene.h"
#include <SimpleMath.h>
#include "TransformComponent.h"
#include "Mesh.h"
#include "MeshPrimitives.h"
#include "MeshComponent.h"
#include "GameObject.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	// D3D12Module init
	m_D3D12Module = std::make_unique<D3D12Module>();
	bool ok = m_D3D12Module->Init(hwnd);
	if (ok)
		Logger::Log("DirectX 12 Initialization success!", LogType::Engine);

	// ResourceModule init
	m_ResourceModule = std::make_unique<ResourceModule>();
	ok = ok && m_ResourceModule->Init(m_D3D12Module.get());

	// ShaderDescriptorModule init
	m_ShaderDescriptorModule = std::make_unique<ShaderDescriptorModule>();
	ok = ok && m_ShaderDescriptorModule->Init(*m_D3D12Module);

	return ok;
}

void Renderer::PreRender()
{
	m_D3D12Module->BeginFrame();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_D3D12Module->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_D3D12Module->GetCommandList()->ResourceBarrier(1, &barrier);
}

void Renderer::Render(Camera& camera, Scene* scene)
{
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
	
	if (scene)
	{
		for (const auto& go : scene->GetGameObjects())
		{
			if (!go->IsActive())
			{
				continue;
			}

			// Find MeshComponent
			Component* comp = go->GetComponent(ComponentType::Mesh);
			if (!comp || !comp->IsEnabled())
			{
				continue;
			}

			// Get Mesh
			Mesh* m = static_cast<MeshComponent*>(comp)->GetMesh();
			if (!m)
			{
				continue;
			}
			
			Matrix model = go->GetTransform()->GetWorldMatrix();
			DirectX::SimpleMath::Matrix mvp = (model * camera.GetView() * camera.GetProjection()).Transpose();

			const auto& vbv = m->GetVertexBufferView();
			const auto& ibv = m->GetIndexBufferView();
			
			m_D3D12Module->GetCommandList()->IASetVertexBuffers(0, 1, &vbv);
			m_D3D12Module->GetCommandList()->IASetIndexBuffer(&ibv);

			m_D3D12Module->GetCommandList()->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(UINT32), &mvp, 0);

			m_D3D12Module->GetCommandList()->DrawIndexedInstanced(m->GetIndexCount(), 1, 0, 0, 0);
		}
	}
}

void Renderer::PostRender()
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_D3D12Module->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_D3D12Module->GetCommandList()->ResourceBarrier(1, &barrier);

	m_D3D12Module->GetCommandList()->Close();

	ID3D12CommandList* commandLists[] = { m_D3D12Module->GetCommandList() };
	m_D3D12Module->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// D3D12 EndFrame
	m_D3D12Module->EndFrame();

	// Flush
	m_D3D12Module->WaitForGPU();
}