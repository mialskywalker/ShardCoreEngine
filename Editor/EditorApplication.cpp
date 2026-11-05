#include "EditorApplication.h"
#include "D3D12Module.h"
#include "ShaderDescriptorModule.h"
#include "imgui_impl_win32.h"

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
}

void EditorApplication::OnRenderOverlay()
{
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); // Enable ImGui Docking
	ImGui::ShowDemoWindow(); // Demo Window
	m_EditorUI.Render(GetRenderer().GetD3D12()->GetCommandList(), *GetRenderer().GetShaderDescriptors()); // Render ImGui
}

bool EditorApplication::CanUpdateCamera() const
{
	if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) // Capture clicks and keys on ImGui windows instead of scene
	{
		return false;
	}

	return true;
}
