#include "Renderer.h"
#include "Logger.h"

#include "D3D12Module.h"
#include "ResourceModule.h"
#include "ShaderDescriptorModule.h"
#include "RenderTexture.h"

#include "Camera.h"
#include "DebugDraw.h"
#include "Scene.h"
#include "Mesh.h"
#include "GameObject.h"

#include "TransformComponent.h"
#include "MeshComponent.h"

#include <SimpleMath.h>
#include <cstdlib>

Renderer::Renderer() {}

Renderer::~Renderer() 
{
	if (m_DebugDraw)
	{
		m_DebugDraw->Shutdown();
	}
}

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

	// RenderTexture init
	m_SceneRenderTexture = std::make_unique<RenderTexture>();
	ok = ok && m_SceneRenderTexture->Init(*m_D3D12Module, *m_ShaderDescriptorModule, m_D3D12Module->GetWidth(), m_D3D12Module->GetHeight());

	// DebugDraw init
	m_DebugDraw = std::make_unique<DebugDraw>();
	ok = ok && m_DebugDraw->Init(*m_D3D12Module);

	return ok;
}

void Renderer::PreRender()
{
	if (m_RequestedSceneWidth > 0 && m_RequestedSceneHeight > 0)
	{
		if (m_SceneRenderTexture->GetWidth() != m_RequestedSceneWidth || m_SceneRenderTexture->GetHeight() != m_RequestedSceneHeight)
		{
			if (!m_SceneRenderTexture->Resize(m_RequestedSceneWidth, m_RequestedSceneHeight))
			{
				Logger::Error("Scene RenderTexture resize failed", LogType::Engine);
			}
		}

		m_RequestedSceneWidth = 0;
		m_RequestedSceneHeight = 0;
	}

	m_D3D12Module->BeginFrame();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_D3D12Module->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_D3D12Module->GetCommandList()->ResourceBarrier(1, &barrier);
}

void Renderer::Render(Camera& camera, Scene* scene)
{
	RenderSceneToTexture(camera, scene);
	PrepareBackBuffer();
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
	if (!m_D3D12Module->WaitForGPU())
	{
		std::abort();
	}
}

void Renderer::RenderSceneToTexture(Camera& camera, Scene* scene)
{
	auto* commandList = m_D3D12Module->GetCommandList();

	// Shader Resource -> Render Target
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_SceneRenderTexture->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	auto rtv = m_SceneRenderTexture->GetRTV();
	auto dsv = m_SceneRenderTexture->GetDSV();

	// Viewport & Scissor
	float width = static_cast<float>(m_SceneRenderTexture->GetWidth());
	float height = static_cast<float>(m_SceneRenderTexture->GetHeight());
	
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	// Bind Render Target
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	// Clear
	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH,	1.0f, 0, 0, nullptr);

	// Update camera aspectRatio
	float aspectRatio = static_cast<float>(m_SceneRenderTexture->GetWidth()) / static_cast<float>(m_SceneRenderTexture->GetHeight());
	camera.SetAspectRatio(aspectRatio);

	// Draw GameObjects
	DrawScene(camera, scene);

	// Debug Draw
	m_DebugDraw->Render(camera);

	// Render Target -> Shader Resource
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_SceneRenderTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);
}

void Renderer::DrawScene(Camera& camera, Scene* scene)
{
	if (!scene)
	{
		return;
	}
	
	auto* commandList = m_D3D12Module->GetCommandList();
	commandList->SetPipelineState(m_D3D12Module->GetPipelineStateObject());
	commandList->SetGraphicsRootSignature(m_D3D12Module->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

		commandList->IASetVertexBuffers(0, 1, &vbv);
		commandList->IASetIndexBuffer(&ibv);

		commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / sizeof(UINT32), &mvp, 0);

		commandList->DrawIndexedInstanced(m->GetIndexCount(), 1, 0, 0, 0);
	}
}

void Renderer::PrepareBackBuffer()
{
	auto* commandList = m_D3D12Module->GetCommandList();
	auto rtv = m_D3D12Module->GetCurrentRTVHandle();

	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

	commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Renderer::RequestSceneResize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	m_RequestedSceneWidth = width;
	m_RequestedSceneHeight = height;
}