#include "EditorApplication.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "RenderTexture.h"
#include "imgui_impl_win32.h"

#include "DebugDraw.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

EditorApplication::~EditorApplication()
{
	m_EditorUI.Shutdown();
}

bool EditorApplication::Init()
{
	// Initialize Application
	if (!Application::Init())
	{
		return false;
	}

	if (!m_EditorCamera.Init(
		GetWindow().GetWidth(),
		GetWindow().GetHeight()))
	{
		return false;
	}

	// Initialize EditorUI (ImGui)
	if (!m_EditorUI.Init(
		GetWindow().GetHandle(),
		*GetRenderer().GetD3D12(),
		*GetRenderer().GetShaderDescriptors()))
	{
		return false;
	}

	// Set the Message Callback
	GetWindow().SetMessageCallback(
		[](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> bool
		{
			ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
			return false;
		}
	);

	return true;
}

void EditorApplication::OnBeginFrame()
{
	m_EditorUI.BeginFrame();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); // Enable ImGui Docking
	m_EditorUI.Draw(GetActiveScene(), GetRenderer().GetSceneRenderTexture(), m_EditorCamera.GetCamera()); // Draw ImGui Windows
	GetRenderer().RequestSceneResize(m_EditorUI.GetViewportWidth(), m_EditorUI.GetViewportHeight()); // Window resize
}

void EditorApplication::OnRenderOverlay()
{
	m_EditorUI.Render(GetRenderer().GetD3D12()->GetCommandList(), *GetRenderer().GetShaderDescriptors()); // Render ImGui
}

void EditorApplication::OnUpdate(float deltaTime)
{
	m_EditorCamera.Update(deltaTime, m_EditorUI.IsSceneHovered());

	// TEMP
	if (m_EditorUI.GetSettings().ShowGrid)
	{
		GetRenderer().GetDebugDraw()->DrawGrid(20, 1.0f, Vector3(0.3f, 0.3f, 0.3f));
	}
}

Camera& EditorApplication::GetRenderCamera()
{
	return m_EditorCamera.GetCamera();
}